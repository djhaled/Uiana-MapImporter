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
#include "PSKReader.h"
#include "Engine/RendererSettings.h"
#include "PSKXFactory.h"
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
	//auto ChildNodes = Node->AddChildNode()
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
	/// here is old script everythign works down here so dont bother
	//Get the static mesh that we're going to paint
	if (SM)
	{
		//Get the vertex buffer from the 1st lod
		//FPositionVertexBuffer* PositionVertexBuffer = &SM->RenderData->LODResources[0].VertexBuffers.PositionVertexBuffer;

		//Make sure that we have at least 1 LOD
		SMComp->SetLODDataCount(1, SMComp->LODData.Num());
		FStaticMeshComponentLODInfo* LODInfo = &SMComp->LODData[0]; //We're going to modify the 1st LOD only
		FStaticMeshLODResources& LodResources = SM->GetRenderData()->LODResources[0];
		auto numverts = LodResources.GetNumVertices();
		//Empty the painted vertices and assign a new color vertex buffer which will contain the new colors for each vertex
		LODInfo->PaintedVertices.Empty();
		LODInfo->OverrideVertexColors = new FColorVertexBuffer();

		// get reader positions /////
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
		//Since we know beforehand the number of elements we might as well reserve the memory now
		//Initialize the new vertex colros with the array we created above
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

		//Initialize resource and mark render state of object as dirty in order for the engine to re-render it
		BeginInitResource(LODInfo->OverrideVertexColors);
	}
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
	int index = -1;
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
	// Remove the single quotation marks from the resulting path
	PathToGo.RemoveFromEnd("'");
	PathToGo.RemoveFromStart("'");
	PathToGo = PathToGo.Replace(TEXT("StaticMesh'"), TEXT(""));
	//L"/Game/ValorantContent/Meshes/StaticMesh'shared_SkyDomeB'"
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
		auto TexPackage = CreatePackage(nullptr ,*PathForTextures);
		TexPackage->FullyLoad();
		auto bCancelled = false;
		auto NewTxName = TexName.Replace(TEXT(".png"),TEXT(""));
		auto CreatedTexture = TextureFactory->FactoryCreateFile(UTexture2D::StaticClass(), TexPackage, FName(*NewTxName), RF_Public | RF_Standalone, tx, NULL, GWarn, bCancelled); 
		if (CreatedTexture == nullptr)
		{
			continue;
		}
		auto Tex = CastChecked<UTexture2D>(CreatedTexture);
		/// tx 
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
	auto PSKFactory = NewObject<UPSKXFactory>();
	PSKFactory->AutomatedImportData = AutomatedData;
	FScopedSlowTask ImportTask(AllMeshesPath.Num(), FText::FromString("Importing Meshes"));
	ImportTask.MakeDialog(true); 
	int ActorIdx = -1;
	for (FString MPath : AllMeshesPath)
	{
		FString MeshGamePath, MeshName;
		MPath.Split(TEXT("\\"), &MeshGamePath, &MeshName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		ActorIdx++;
		// JSON Stuff

		///// end json stuff
		MeshName = MeshName.Replace(TEXT(".pskx"), TEXT(""));
		FString PathForMeshes = FString::Printf(TEXT("/Game/ValorantContent/Meshes/%s"), *MeshName);
		auto MeshPackage = CreatePackage(*PathForMeshes);
		auto bCancelled = false;
		auto CreatedMesh = PSKFactory->FactoryCreateFile(UStaticMesh::StaticClass(), MeshPackage, FName(*MeshName), RF_Public | RF_Standalone, MPath, NULL, GWarn, bCancelled);
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
		//Msh->Property
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

