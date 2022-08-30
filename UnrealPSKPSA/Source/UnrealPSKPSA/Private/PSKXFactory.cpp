// Fill out your copyright notice in the Description page of Project Settings.


#include "PSKXFactory.h"
#include "ActorXUtils.h"
#include "PSKReader.h"
#include "RawMesh.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Editor/UnrealEd/Classes/Factories/MaterialInstanceConstantFactoryNew.h"
#include "EditorAssetLibrary.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Misc/FileHelper.h"
#include "Materials/MaterialInstanceConstant.h"

UObject* UPSKXFactory::Import(const FString Filename, UObject* Parent, const FName Name, const EObjectFlags Flags) const
{
	const auto Reader = new PSKReader(Filename);
	if (!Reader->Read())
	{
		return nullptr;
	}
	auto bHasNormals = Reader->Normals.Num() > 0;
	auto bHasVertexColors = Reader->VertexColors.Num() > 0;

	TArray<FColor> FaceVertexColors;
	FaceVertexColors.Init(FColor::White, Reader->VertexColors.Num());
	if (bHasVertexColors)
	{
		for (auto i = 0; i < Reader->Wedges.Num(); i++)
		{
			auto FixedColor = Reader->VertexColors[i];
			Swap(FixedColor.R, FixedColor.B);
			FaceVertexColors[Reader->Wedges[i].PointIndex] = FixedColor;
		}
	}
	auto RawMesh = FRawMesh();
	for (auto Vertex : Reader->Vertices)
	{
		auto FixedVertex = Vertex;
		FixedVertex.Y = -FixedVertex.Y; // mirror y axis cuz ue dumb dumb
		RawMesh.VertexPositions.Add(FixedVertex);
	}

	for (const auto Face : Reader->Faces)
	{
		for (auto VtxIdx = 2; VtxIdx >= 0; VtxIdx--) // reverse face winding to account for -y vertex pos
		{
			const auto Wedge = Reader->Wedges[Face.WedgeIndex[VtxIdx]];

			RawMesh.WedgeIndices.Add(Wedge.PointIndex);
			RawMesh.WedgeTexCoords[0].Add(FVector2f(Wedge.U, Wedge.V));

			for (auto UVIdx = 0; UVIdx < Reader->ExtraUVs.Num(); UVIdx++)
			{
				auto UV = Reader->ExtraUVs[UVIdx][Face.WedgeIndex[VtxIdx]];
				RawMesh.WedgeTexCoords[UVIdx+1].Add(UV);
			}

			auto Normal = FVector3f::ZeroVector;

			if (bHasNormals)
			{
				Normal = Reader->Normals[Wedge.PointIndex];
				Normal.Y = -Normal.Y;
			}

			RawMesh.WedgeTangentZ.Add(Normal);
			RawMesh.WedgeTangentY.Add(FVector3f::ZeroVector);
			RawMesh.WedgeTangentX.Add(FVector3f::ZeroVector);

			if (bHasVertexColors) RawMesh.WedgeColors.Add(FaceVertexColors[Wedge.PointIndex]);
		}
		
		RawMesh.FaceMaterialIndices.Add(Face.MatIndex);
		RawMesh.FaceSmoothingMasks.Add(1);
	}

	const auto StaticMesh = CastChecked<UStaticMesh>(CreateOrOverwriteAsset(UStaticMesh::StaticClass(), Parent, Name, Flags));
	for (auto i = 0; i < Reader->Materials.Num(); i++)
	{
		auto Material = Reader->Materials[i];
		auto MaterialName = FName(Material.MaterialName);
		auto MaterialNamey = FString(Material.MaterialName);
		FString NewPathMat = "/Game/ValorantContent/Materials/";
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
		auto singpath = FPaths::Combine(NewPathMat, MaterialNamey);
		UObject* CheckAsset = UEditorAssetLibrary::LoadAsset(singpath);
		if (CheckAsset == nullptr)
		{
			CheckAsset = AssetTools.CreateAsset(MaterialNamey, NewPathMat, UMaterialInstanceConstant::StaticClass(), Factory);
			TArray< FStringFormatArg > args;
			args.Add(FStringFormatArg(MaterialNamey));
			FString PathMat = FString::Format(TEXT("/Game/ValorantContent/Materials/{0}.{0}"), args);
			auto Asset = UEditorAssetLibrary::LoadAsset(PathMat);
			UMaterialInstanceConstant* MaterialInstance = CastChecked<UMaterialInstanceConstant>(Asset);
			MaterialInstance->MarkPackageDirty();
			FStaticMaterial StaticMaterial;
			StaticMaterial.MaterialInterface = MaterialInstance;
			StaticMesh->GetStaticMaterials().Add(StaticMaterial);
			StaticMesh->GetSectionInfoMap().Set(0, i, FMeshSectionInfo(i));
			continue;
		}
		UMaterialInstanceConstant* MaterialInstance = CastChecked<UMaterialInstanceConstant>(CheckAsset);
		MaterialInstance->MarkPackageDirty();
		FStaticMaterial StaticMaterial;
		StaticMaterial.MaterialInterface = MaterialInstance;
		StaticMesh->GetStaticMaterials().Add(StaticMaterial);
		StaticMesh->GetSectionInfoMap().Set(0, i, FMeshSectionInfo(i));
	}
	
	auto& SourceModel = StaticMesh->AddSourceModel();
	SourceModel.BuildSettings.bBuildReversedIndexBuffer = false;
	SourceModel.BuildSettings.bRecomputeTangents = false;
	SourceModel.BuildSettings.bGenerateLightmapUVs = false;
	SourceModel.BuildSettings.bComputeWeightedNormals = false;
	SourceModel.BuildSettings.bRecomputeNormals = !bHasNormals;
	SourceModel.SaveRawMesh(RawMesh);

	StaticMesh->Build();
	StaticMesh->PostEditChange();
	FAssetRegistryModule::AssetCreated(StaticMesh);
	StaticMesh->MarkPackageDirty();
	return StaticMesh;
}


