// Fill out your copyright notice in the Description page of Project Settings.


#include "BPFL.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "AssetToolsModule.h"
#include "Engine/SCS_Node.h"
#include "json.hpp"
#include "KismetProceduralMeshLibrary.h"
#include "AutomatedAssetImportData.h"
#include "MeshPaintAdapterFactory.h"
#include "Engine/World.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/FileHelper.h"
#include "Factories/TextureFactory.h"
#include "StaticMeshDescription.h"
#include "Misc/ScopedSlowTask.h"
#include "PSKReader.h"
#include "Engine/RendererSettings.h"
#include "PSKXFactory.h"
#if ENGINE_MAJOR_VERSION == 5
#include "VectorTypes.h"
#include "AssetRegistry/AssetRegistryModule.h"
#else
#include "AssetRegistryModule.h"
#endif




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
#if ENGINE_MAJOR_VERSION == 5
	return Component.Get();
#else
	return Component;
#endif
}
void UBPFL::PaintSMVertices(UStaticMeshComponent* SMComp, TArray<FColor> VtxColorsArray, FString FileName)
{
	SMComp->SetLODDataCount(1, SMComp->LODData.Num());
	TSharedPtr<IMeshPaintGeometryAdapter> MeshPainter = FMeshPaintAdapterFactory::CreateAdapterForMesh(SMComp, 0);
	UStaticMesh* SM = SMComp->GetStaticMesh();
	if (MeshPainter.IsValid() && SM)
	{
		// get reader positions /////
		TArray<FColor> FinalColors;
		const auto Reader = new PSKReader(FileName);
		Reader->Read();
		TArray<FVector3f> CurrentVerts = ReturnCurrentVerts(SM);
		if (VtxColorsArray.Num() != CurrentVerts.Num())
		{
			FinalColors = FixBrokenMesh(SM,FileName,VtxColorsArray, Reader->Vertices);
		}
		else
		{
			auto Hashmap = MakeHashmap(Reader->Vertices, VtxColorsArray);
			// Shell_3_AtkCourtyardGroundA
			// Shell_3_AtkPathB
			for (int vertIndex = 0; vertIndex < CurrentVerts.Num(); vertIndex++)
			{
				CurrentVerts[vertIndex].Y = -CurrentVerts[vertIndex].Y;
				auto idaa = FVector3f(CurrentVerts[vertIndex]);
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
		if (FinalColors.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("This one has no FinalColors %s"), *SM->GetName());
			return;
		}
		for (int colorIndex = 0; colorIndex < FinalColors.Num(); colorIndex++)
		{
			MeshPainter->SetVertexColor(colorIndex, FinalColors[colorIndex]);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Provided StaticMesh could not be retrieved! Missing %s"), MeshPainter.IsValid() ? TEXT("StaticMesh!") : TEXT("MeshPainter!"));
	}
	MeshPainter.Reset();
}

FColor UBPFL::ReturnFromHex(FString Beka)
{
	return FColor::FromHex(Beka);
}

TMap<FVector3f, FColor> UBPFL::MakeHashmap(TArray<FVector3f> arr1, TArray<FColor> TestVtx)
{
	//arr1 should be correct reader
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
	// Get Actual Verts
	auto CurrentVerticesPosition = ReturnCurrentVerts(SMesh);
	int32  ida = 0;
	for (auto vtx : ReaderVerts)
	{
		ReaderVerts[ida].Y = -vtx.Y;
		ida++;
	}
	TArray<FColor> VtxOrderedColors;
	// from the new array without the missing ones if  equals the vtxpos and hasnt been changed yet add to the vtx colors at the index "correct
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
				//This is in the Static Mesh Actor Class, so it is location and tranform of the SMActor
				const FVector3f Vertex = VertexBuffer->VertexPosition(Index);
				//add to output FVector array
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
#if ENGINE_MAJOR_VERSION == 5
	Settings->DynamicGlobalIllumination = EDynamicGlobalIlluminationMethod::None;
	Settings->Reflections == EReflectionMethod::None;
#endif
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
	for (const FString texturePath : AllTexturesPath)
	{
		ActorIdx++;
		const FString TexName = FPaths::GetBaseFilename(texturePath);
		const FString TexGamePath = FPaths::GetPath(texturePath);
		FString PathForTextures = FString::Printf(TEXT("/Game/ValorantContent/Textures/%s"), *TexName);
		auto TexPackage = CreatePackage(*PathForTextures);
		auto bCancelled = false;
		auto CreatedTexture = TextureFactory->FactoryCreateFile(UTexture2D::StaticClass(), TexPackage, FName(*TexName), RF_Public | RF_Standalone, texturePath, NULL, GWarn, bCancelled); 
		if (CreatedTexture == nullptr)
		{
			continue;
		}
		auto Tex = CastChecked<UTexture2D>(CreatedTexture);
		/// tx 
		auto CompressionSetting = Tex->CompressionSettings;
		if (TexName.EndsWith("MRA") && CompressionSetting != TC_Masks)
		{
			Tex->SRGB = false;
			Tex->CompressionSettings = TC_Masks;
		}
		if (TexName.EndsWith("NM") && CompressionSetting != TC_Normalmap)
		{
			Tex->SRGB = false;
			Tex->CompressionSettings = TC_Normalmap;
		}
		if (TexName.EndsWith("DF") && CompressionSetting != TC_Normalmap)
		{
			Tex->SRGB = true;
			Tex->CompressionSettings = TC_Default;
		}
		ImportTask.DefaultMessage = FText::FromString(FString::Printf(TEXT("Importing Texture : %d of %d: %s"), ActorIdx + 1, AllTexturesPath.Num() + 1, *TexName));
		ImportTask.EnterProgressFrame();
		Tex->MarkPackageDirty();
		FAssetRegistryModule::AssetCreated(Tex);
		Tex->PreEditChange(nullptr);
		Tex->PostEditChange();
	}
}

void UBPFL::ImportMeshes(TSet<FString> AllMeshesPath, FString ObjectsPath)
{
	auto AutomatedData = NewObject<UAutomatedAssetImportData>();
	AutomatedData->bReplaceExisting = false;
	auto PSKFactory = NewObject<UPSKXFactory>();
	PSKFactory->AutomatedImportData = AutomatedData;
	FScopedSlowTask ImportTask(AllMeshesPath.Num(), FText::FromString("Importing Meshes"));
	ImportTask.MakeDialog(true); 
	int ActorIdx = -1;
	for (FString MPath : AllMeshesPath)
	{
		const FString MeshName = FPaths::GetBaseFilename(MPath);
		const FString MeshGamePath = FPaths::GetPath(MPath);
		ActorIdx++;
		FString PathForMeshes = FString::Printf(TEXT("/Game/ValorantContent/Meshes/%s"), *MeshName);
		auto MeshPackage = CreatePackage(*PathForMeshes);
		auto bCancelled = false;
		auto CreatedMesh = PSKFactory->FactoryCreateFile(UStaticMesh::StaticClass(), MeshPackage, FName(*MeshName), RF_Public | RF_Standalone, MPath, NULL, GWarn, bCancelled);
		if (CreatedMesh == nullptr)
		{
			continue;
		}
		ImportTask.DefaultMessage = FText::FromString(FString::Printf(TEXT("Importing Mesh : %d of %d: %s"), ActorIdx + 1, AllMeshesPath.Num() + 1, *MeshName));
		ImportTask.EnterProgressFrame();
	}
}

ECollisionTraceFlag UBPFL::GetTraceFlag(FString tflag)
{
	//CTF_UseComplexAsSimple
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

