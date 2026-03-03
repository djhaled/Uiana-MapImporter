// Fill out your copyright notice in the Description page of Project Settings.


#include "BPFL.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "VectorTypes.h"
#include "Engine/StaticMesh.h"
#include "AssetToolsModule.h"
#include "Engine/SCS_Node.h"
#include "json.hpp"
#include "KismetProceduralMeshLibrary.h"
#include "AutomatedAssetImportData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/World.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/FileHelper.h"
#include "EditorAssetLibrary.h"
#include "Factories/TextureFactory.h"
#include "StaticMeshDescription.h"
#include "Misc/ScopedSlowTask.h"
#include "Engine/RendererSettings.h"
#include "Readers/UEFModelReader.h"
#include "Factories/UEFModelFactory.h"
#include "StaticMeshComponentLODInfo.h"
#include "UObject/SavePackage.h"


UActorComponent* UBPFL::GetComponentByName(AActor* Actor, FName CompName)
{
	UStaticMeshComponent* Trolley = Cast<UStaticMeshComponent>(Actor->GetDefaultSubobjectByName(CompName));
	if (!Trolley)
	{
		return nullptr;
	}
	return Trolley;
}
void UBPFL::SetOverrideMaterial(AActor* Actor, FName CompName, TArray<UMaterialInterface*> MatOvr)
{
	UStaticMeshComponent* Trolley = Cast<UStaticMeshComponent>(Actor->GetDefaultSubobjectByName(CompName));
	if (!Trolley)
	{
		return;
	}
	Trolley->OverrideMaterials = MatOvr;
}
USCS_Node* UBPFL::CreateNode(UObject* Object, UClass* ClassToUse, FName CompName,UActorComponent*& ComponentReturn)
{
	IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UBlueprint* Blueprint = Cast<UBlueprint>(Object);
	USCS_Node* Node = Blueprint->SimpleConstructionScript->CreateNode(ClassToUse, CompName);
	ComponentReturn = Node->ComponentTemplate;
	return Node;
}
UActorComponent* UBPFL::CreateBPComp(UObject* Object, UClass* ClassToUse, FName CompName,TArray<USCS_Node*> AttachNodes)
{
	IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UBlueprint* Blueprint = Cast<UBlueprint>(Object);
	USCS_Node* Node = Blueprint->SimpleConstructionScript->CreateNode(ClassToUse, CompName);
	auto Component = Node->ComponentTemplate;
	Blueprint->SimpleConstructionScript->AddNode(Node);
	for (auto AttchNode : AttachNodes)
	{
		Node->AddChildNode(AttchNode);
	}
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	return Component.Get();
}
void UBPFL::PaintSMVertices(UStaticMeshComponent* SMComp, TArray<FColor> VtxColorsArray, FString FileName)
{
	TArray<FColor> FinalColors;
	UStaticMesh* SM = SMComp->GetStaticMesh();
	if (SM)
	{
		SMComp->SetLODDataCount(1, SMComp->LODData.Num());
		FStaticMeshComponentLODInfo* LODInfo = &SMComp->LODData[0];
		FStaticMeshLODResources& LodResources = SM->GetRenderData()->LODResources[0];
		LODInfo->PaintedVertices.Empty();
		LODInfo->OverrideVertexColors = new FColorVertexBuffer();

		UEFModelReader Reader(FileName);
		if (!Reader.Read() || Reader.LODs.Num() == 0) return;
		const TArray<FVector3f>& ReaderVerts = Reader.LODs[0].Vertices;
		TArray<FVector3f> CurrentVerts = ReturnCurrentVerts(SM);
		if (VtxColorsArray.Num() != CurrentVerts.Num())
		{
			FinalColors = FixBrokenMesh(SM, FileName, VtxColorsArray, ReaderVerts);
		}
		else
		{
			auto Hashmap = MakeHashmap(ReaderVerts, VtxColorsArray);
			for (auto vt : CurrentVerts)
			{
				vt.Y = -vt.Y;
				auto idaa = FVector3f(vt);
				auto finder = Hashmap.Find(idaa);
				if (finder)
				{
					FinalColors.Add(*finder);
				}
			}
		}
		if (FinalColors.Num() != CurrentVerts.Num())
		{
			UE_LOG(LogTemp, Warning, TEXT("This one has wrong FinalColors %s"), *SM->GetName());
		}
		if (FinalColors.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("This one has no  FinalColors %s"), *SM->GetName());
			return;
		}
		LODInfo->OverrideVertexColors->InitFromColorArray(FinalColors);
		BeginInitResource(LODInfo->OverrideVertexColors);
	}
}

FColor UBPFL::ReturnFromHex(FString Beka)
{
	return FColor::FromHex(Beka);
}

TMap<FVector3f, FColor> UBPFL::MakeHashmap(TArray<FVector3f> arr1, TArray<FColor> TestVtx)
{
	TMap<FVector3f, FColor> FruitMap;
	FruitMap.Empty();
	int idx = 0;
	for (auto j : arr1)
	{
		FruitMap.Add(j, TestVtx[idx]);
		idx++;
	}
	return FruitMap;
}


TArray<FColor> UBPFL::FixBrokenMesh(UStaticMesh* SMesh, FString ReaderFile, TArray<FColor> BrokenVtxColorArray,TArray<FVector3f> ReaderVerts)
{
	TArray<FColor> LocalVtxColors;
	auto CurrentVerticesPosition = ReturnCurrentVerts(SMesh);
	int32  ida = 0;
	for (auto vtx : ReaderVerts)
	{
		ReaderVerts[ida].Y = -vtx.Y;
		ida++;
	}
	auto Hasher = MakeHashmap(ReaderVerts, BrokenVtxColorArray);
	for (auto vt : CurrentVerticesPosition)
	{
		auto finder = Hasher.Find(FVector3f(vt));
		LocalVtxColors.Add(*finder);
	}
	return LocalVtxColors;
}

TArray<FVector3f> UBPFL::ReturnCurrentVerts(UStaticMesh* Mesh)
{
	TArray<FVector3f> ReturnArray;
	if (Mesh->GetRenderData()->LODResources.Num() > 0)
	{
		FPositionVertexBuffer* VertexBuffer = &Mesh->GetRenderData()->LODResources[0].VertexBuffers.PositionVertexBuffer;
		if (VertexBuffer)
		{
			const int32 VertexCount = VertexBuffer->GetNumVertices();
			for (int32 Index = 0; Index < VertexCount; Index++)
			{
				const FVector3f Vertex = VertexBuffer->VertexPosition(Index);
				ReturnArray.Add(Vertex);
			}
		}
	}
	return ReturnArray;
}
void UBPFL::ChangeProjectSettings()
{
	URendererSettings* Settings = GetMutableDefault<URendererSettings>();
	Settings->DefaultLightUnits = ELightUnits::Unitless;
	Settings->DynamicGlobalIllumination = EDynamicGlobalIlluminationMethod::None;
	Settings->Reflections == EReflectionMethod::None;
	Settings->SaveConfig();
}
UActorComponent* UBPFL::GetComponent(AActor* Actor)
{
	Actor->SpriteScale;
	UActorComponent* RootComp = Actor->GetRootComponent();
	return Actor->GetRootComponent();
}
void UBPFL::ExecuteConsoleCommand(FString ConsoleCommand) {
	if (GEditor) {
		UWorld* World = GEditor->GetEditorWorldContext().World();
		if (World) {
			GEditor->Exec(World, *ConsoleCommand, *GLog);
		}
	}
}
UObject* UBPFL::SetMeshReference(FString MeshObjectName, FString MeshType)
{
	FString PathToGo = FString::Printf(TEXT("/Game/ValorantContent/%s/%s"), *MeshType, *MeshObjectName);
	PathToGo.RemoveFromEnd("'");
	PathToGo.RemoveFromStart("'");
	PathToGo = PathToGo.Replace(TEXT("StaticMesh'"), TEXT(""));
	auto Asset = UEditorAssetLibrary::LoadAsset(PathToGo);
	return Asset;
}

void UBPFL::ImportTextures(TArray<FString> AllTexturesPath)
{
	auto AutomatedData = NewObject<UAutomatedAssetImportData>();
	AutomatedData->bReplaceExisting = false;
	auto TextureFactory = NewObject<UTextureFactory>();
	TextureFactory->NoCompression = true;
	TextureFactory->AutomatedImportData = AutomatedData;
	FScopedSlowTask ImportTask(AllTexturesPath.Num(), FText::FromString("Importing Textures"));
	ImportTask.MakeDialog(true);
	auto ActorIdx = -1;
	for (auto tx : AllTexturesPath)
	{
		ActorIdx++;
		FString TexGamePath, TexName;
		tx.Split(TEXT("\\"), &TexGamePath, &TexName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		FString PathForTextures = FString::Printf(TEXT("/Game/ValorantContent/Textures/%s"), *TexName.Replace(TEXT(".png"),TEXT("")));
		auto TexPackage = CreatePackage(*PathForTextures);
		TexPackage->FullyLoad();
		auto bCancelled = false;
		auto NewTxName = TexName.Replace(TEXT(".png"),TEXT(""));
		auto CreatedTexture = TextureFactory->FactoryCreateFile(UTexture2D::StaticClass(), TexPackage, FName(*NewTxName), RF_Public | RF_Standalone, tx, NULL, GWarn, bCancelled); 
		if (CreatedTexture == nullptr)
		{
			continue;
		}
		auto Tex = CastChecked<UTexture2D>(CreatedTexture);
		auto CompressionSetting = Tex->CompressionSettings;
		if (NewTxName.EndsWith("MRA") && CompressionSetting != TC_Masks)
		{
			Tex->SRGB = false;
			Tex->CompressionSettings = TC_Masks;
		}
		if (NewTxName.EndsWith("NM") && CompressionSetting != TC_Normalmap)
		{
			Tex->SRGB = false;
			Tex->CompressionSettings = TC_Normalmap;
		}
		if (NewTxName.EndsWith("DF") && CompressionSetting != TC_Normalmap)
		{
			Tex->SRGB = true;
			Tex->CompressionSettings = TC_Default;
		}
		ImportTask.DefaultMessage = FText::FromString(FString::Printf(TEXT("Importing Texture : %d of %d: %s"), ActorIdx + 1, AllTexturesPath.Num() + 1, *NewTxName));
		ImportTask.EnterProgressFrame();
		
		Tex->UpdateResource();
		FAssetRegistryModule::AssetCreated(Tex);
		const FString PackageFileName = FPackageName::LongPackageNameToFilename(TexPackage->GetName(), FPackageName::GetAssetPackageExtension());
		FSavePackageArgs SaveArgs;
		UPackage::SavePackage(TexPackage, Tex, *PackageFileName, SaveArgs);
	}
}

void UBPFL::ImportMeshes(TArray<FString> AllMeshesPath, FString ObjectsPath)
{
	auto AutomatedData = NewObject<UAutomatedAssetImportData>();
	AutomatedData->bReplaceExisting = false;
	auto UEFModelFactory = NewObject<UUEFModelFactory>();
	UEFModelFactory->AutomatedImportData = AutomatedData;
	FScopedSlowTask ImportTask(AllMeshesPath.Num(), FText::FromString("Importing Meshes"));
	ImportTask.MakeDialog(true); 
	int ActorIdx = -1;
	for (FString MPath : AllMeshesPath)
	{
		FString MeshGamePath, MeshName;
		MPath.Split(TEXT("\\"), &MeshGamePath, &MeshName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		ActorIdx++;
		MeshName = MeshName.Replace(TEXT(".uemodel"), TEXT(""));
		FString PathForMeshes = FString::Printf(TEXT("/Game/ValorantContent/Meshes/%s"), *MeshName);
		auto MeshPackage = CreatePackage(*PathForMeshes);
		auto bCancelled = false;
		auto CreatedMesh = UEFModelFactory->FactoryCreateFile(UStaticMesh::StaticClass(), MeshPackage, FName(*MeshName), RF_Public | RF_Standalone, MPath, NULL, GWarn, bCancelled);
		if (CreatedMesh == nullptr)
		{
			continue;
		}
		FAssetRegistryModule::AssetCreated(CreatedMesh);
		FSavePackageArgs SaveArgs;
		const FString PackageFileName = FPackageName::LongPackageNameToFilename(MeshPackage->GetName(), FPackageName::GetAssetPackageExtension());
		UPackage::SavePackage(MeshPackage, nullptr, *PackageFileName, SaveArgs);
		ImportTask.DefaultMessage = FText::FromString(FString::Printf(TEXT("Importing Mesh : %d of %d: %s"), ActorIdx + 1, AllMeshesPath.Num() + 1, *MeshName));
		ImportTask.EnterProgressFrame();
	}
}

ECollisionTraceFlag UBPFL::GetTraceFlag(FString tflag)
{
	if (tflag == "CTF_UseDefault")
	{
		return CTF_UseDefault;
	}
	if (tflag == "CTF_UseSimpleAndComplex")
	{
		return CTF_UseSimpleAndComplex;
	}
	if (tflag == "CTF_UseSimpleAsComplex")
	{
		return CTF_UseSimpleAsComplex;
	}
	if (tflag == "CTF_UseComplexAsSimple")
	{
		return CTF_UseComplexAsSimple;
	}
	if (tflag == "CTF_MAX")
	{
		return CTF_MAX;
	}
	return CTF_UseSimpleAsComplex;
}
