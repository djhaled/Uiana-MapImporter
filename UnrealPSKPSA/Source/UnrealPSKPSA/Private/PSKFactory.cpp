#include "PSKFactory.h"
#include "json.hpp"
#include "PSKReader.h"
#include "RawMesh.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor/UnrealEd/Classes/Factories/MaterialInstanceConstantFactoryNew.h"
#include "EditorAssetLibrary.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Misc/FileHelper.h"
#include "Materials/MaterialInstanceConstant.h"

DEFINE_LOG_CATEGORY(LogPSKPSA);
UPSKFactory::UPSKFactory()
{
	bCreateNew = false;
	bEditorImport = true;
	bText = false;

	Formats.Add(TEXT("pskx;ActorX Static Mesh"));
	Formats.Add(TEXT("psk;ActorX Skeletal Mesh"));

	SupportedClass = UStaticMesh::StaticClass();
}

bool UPSKFactory::DoesSupportClass(UClass* InClass)
{
	return InClass == UStaticMesh::StaticClass();
}

UClass* UPSKFactory::ResolveSupportedClass()
{
	return UStaticMesh::StaticClass();
}

bool UPSKFactory::FactoryCanImport(const FString& InSystemFilePath)
{
	const auto Extension = FPaths::GetExtension(InSystemFilePath);
	const auto bIsSkeletal = Extension.Compare("psk", ESearchCase::IgnoreCase) == 0;
	const auto bIsStatic = Extension.Compare("pskx", ESearchCase::IgnoreCase) == 0;
	return bIsSkeletal || bIsStatic;
}

UObject* UPSKFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	const auto Extension = FPaths::GetExtension(Filename);

	if (Extension.Equals("pskx"))
	{
		return ImportPSKX(Filename, InParent, InName, Flags);
	}
	if (Extension.Equals("psk"))
	{
		return ImportPSKX(Filename, InParent, InName, Flags);
	}
	
	return nullptr;
}

UObject* UPSKFactory::ImportPSKX(const FString Filename, UObject* Parent, const FName Name, const EObjectFlags Flags)
{
	auto Reader = PskReader(Filename);
	Reader.Read();

	TArray<FColor> VertexColorsByFace;
	if (Reader.VertexColors.size() > 0)
	{
		VertexColorsByFace.Init(FColor(0,0,0,0), Reader.VertexColors.size());
		for (auto i = 0; i < Reader.Wedges.size(); i++)
		{
			VertexColorsByFace[Reader.Wedges[i].PointIndex] = Reader.VertexColors[i];
		}
	}	

	auto RawMesh = FRawMesh();
	for (auto Vert : Reader.Vertices)
	{
		//Vert.Y = -Vert.Y;
		RawMesh.VertexPositions.Add(Vert);
	}
	for (auto Face : Reader.Faces)
	{
		for (auto VertexIndex = 0; VertexIndex < 3; VertexIndex++)
		{
			auto Wedge = Reader.Wedges[Face.WedgeIndex[VertexIndex]];
			
			RawMesh.WedgeIndices.Add(Wedge.PointIndex);
			RawMesh.WedgeTexCoords[0].Add(FVector2f(Wedge.U, Wedge.V));
			
			for (auto i = 0; i < Reader.ExtraUVs.size(); i++)
			{
				auto UV = Reader.ExtraUVs[i].UVData[Face.WedgeIndex[i]];
				RawMesh.WedgeTexCoords[i+1].Add(UV);
			}

			if (VertexColorsByFace.Num() > 0)
			{
				RawMesh.WedgeColors.Add(VertexColorsByFace[Wedge.PointIndex]);
			}

			if (Reader.Normals.size() > 0)
			{
				RawMesh.WedgeTangentZ.Add(Reader.Normals[Wedge.PointIndex]);
			}
			else
			{
				RawMesh.WedgeTangentZ.Add(FVector3f::ZeroVector);
			}

			RawMesh.WedgeTangentY.Add(FVector3f::ZeroVector);
			RawMesh.WedgeTangentX.Add(FVector3f::ZeroVector);
		}
		
		RawMesh.FaceMaterialIndices.Add(Face.MatIndex);
		RawMesh.FaceSmoothingMasks.Add(1);
	}

	const auto StaticMesh = CastChecked<UStaticMesh>(CreateOrOverwriteAsset(UStaticMesh::StaticClass(), Parent, Name, Flags));
	for (auto MatIndex = 0; MatIndex < Reader.Materials.size(); MatIndex++)
	{
		auto MaterialName = FName(Reader.Materials[MatIndex].MaterialName);
		auto MaterialNamey = FString(Reader.Materials[MatIndex].MaterialName);
		FString Test = "/Game/Meshes/All/";
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
		auto lala = AssetTools.CreateAsset(MaterialNamey, Test, UMaterialInstanceConstant::StaticClass(), Factory);
		if (lala == nullptr)
		{
			TArray< FStringFormatArg > args;
			args.Add(FStringFormatArg(MaterialNamey));
			FString PathMat = FString::Format(TEXT("/Game/Meshes/All/{0}.{0}"), args);
			auto Asset = UEditorAssetLibrary::LoadAsset(PathMat);
			UMaterialInstanceConstant* MaterialInstance = CastChecked<UMaterialInstanceConstant>(Asset);
			MaterialInstance->MarkPackageDirty();
			FStaticMaterial StaticMaterial;
			StaticMaterial.MaterialInterface = MaterialInstance;
			StaticMesh->GetStaticMaterials().Add(StaticMaterial);
			StaticMesh->GetSectionInfoMap().Set(0, MatIndex, FMeshSectionInfo(MatIndex));
			continue;
		}
		UMaterialInstanceConstant* MaterialInstance = CastChecked<UMaterialInstanceConstant>(lala);
		MaterialInstance->MarkPackageDirty();
		FStaticMaterial StaticMaterial;
		StaticMaterial.MaterialInterface = MaterialInstance;
		StaticMesh->GetStaticMaterials().Add(StaticMaterial);
		StaticMesh->GetSectionInfoMap().Set(0, MatIndex, FMeshSectionInfo(MatIndex));
	}

	auto& SourceModel = StaticMesh->AddSourceModel();
	SourceModel.BuildSettings.bGenerateLightmapUVs = false;
	SourceModel.BuildSettings.bBuildReversedIndexBuffer = false;
	SourceModel.BuildSettings.bRecomputeTangents = true;
	SourceModel.BuildSettings.bComputeWeightedNormals = true;
	SourceModel.BuildSettings.bRecomputeNormals = Reader.Normals.size() == 0;
	SourceModel.SaveRawMesh(RawMesh);

	StaticMesh->Build();
	StaticMesh->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(StaticMesh);
	
	return StaticMesh;
}




