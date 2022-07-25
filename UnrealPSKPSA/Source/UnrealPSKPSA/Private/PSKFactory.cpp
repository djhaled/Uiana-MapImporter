// Fill out your copyright notice in the Description page of Project Settings.


#include "PSKFactory.h"

#include "ActorXUtils.h"
#include "IMeshBuilderModule.h"
#include "MeshDescription.h"
#include "PSKReader.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ImportUtils/SkeletalMeshImportUtils.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "Rendering/SkeletalMeshModel.h"


UObject* UPSKFactory::Import(const FString Filename, UObject* Parent, const FName Name, const EObjectFlags Flags) const
{
	auto Psk = PSKReader(Filename);
	if (!Psk.Read()) return nullptr;

	TArray<FColor> PointVertexColors;
	PointVertexColors.Init(FColor::White, Psk.VertexColors.Num());
	if (Psk.bHasVertexColors)
	{
		for (auto i = 0; i < Psk.Wedges.Num(); i++)
		{
			auto FixedColor = Psk.VertexColors[i];
			Swap(FixedColor.R, FixedColor.B);
			PointVertexColors[Psk.Wedges[i].PointIndex] = FixedColor;
		}
	}
	
	FSkeletalMeshImportData SkeletalMeshImportData;

	for (auto i = 0; i < Psk.Normals.Num(); i++)
		Psk.Normals[i].Y = -Psk.Normals[i].Y;

	for (auto Vertex : Psk.Vertices)
	{
		auto FixedVertex = Vertex;
		FixedVertex.Y = -FixedVertex.Y;
		SkeletalMeshImportData.Points.Add(FixedVertex);
		SkeletalMeshImportData.PointToRawMap.Add(SkeletalMeshImportData.Points.Num()-1);
	}
	
	auto WindingOrder = {2, 1, 0};
	for (const auto PskFace : Psk.Faces)
	{
		SkeletalMeshImportData::FTriangle Face;
		Face.MatIndex = PskFace.MatIndex;
		Face.SmoothingGroups = 1;
		Face.AuxMatIndex = 0;

		for (auto VertexIndex : WindingOrder)
		{
			const auto WedgeIndex = PskFace.WedgeIndex[VertexIndex];
			const auto PskWedge = Psk.Wedges[WedgeIndex];
			
			SkeletalMeshImportData::FVertex Wedge;
			Wedge.MatIndex = PskWedge.MatIndex;
			Wedge.VertexIndex = PskWedge.PointIndex;
			Wedge.Color = Psk.bHasVertexColors ? PointVertexColors[PskWedge.PointIndex] : FColor::White;
			Wedge.UVs[0] = FVector2f(PskWedge.U, PskWedge.V);
			for (auto UVIdx = 0; UVIdx < Psk.ExtraUVs.Num(); UVIdx++)
			{
				auto UV =  Psk.ExtraUVs[UVIdx][Face.WedgeIndex[VertexIndex]];
				Wedge.UVs[UVIdx+1] = UV;
			}
			
			Face.WedgeIndex[VertexIndex] = SkeletalMeshImportData.Wedges.Add(Wedge);
			Face.TangentZ[VertexIndex] = Psk.bHasVertexNormals ? Psk.Normals[PskWedge.PointIndex] : FVector3f::ZeroVector;
			Face.TangentY[VertexIndex] = FVector3f::ZeroVector;
			Face.TangentX[VertexIndex] = FVector3f::ZeroVector;

			
		}
		Swap(Face.WedgeIndex[0], Face.WedgeIndex[2]);
		Swap(Face.TangentZ[0], Face.TangentZ[2]);

		SkeletalMeshImportData.Faces.Add(Face);
	}

	for (auto PskBone : Psk.Bones)
	{
		SkeletalMeshImportData::FBone Bone;
		Bone.Name = PskBone.Name;
		Bone.NumChildren = PskBone.NumChildren;
		Bone.ParentIndex = PskBone.ParentIndex == -1 ? INDEX_NONE : PskBone.ParentIndex;
		
		auto PskBonePos = PskBone.BonePos;
		FTransform3f PskTransform;
		PskTransform.SetLocation(FVector3f(PskBonePos.Position.X, -PskBonePos.Position.Y, PskBonePos.Position.Z));
		PskTransform.SetRotation(FQuat4f(PskBonePos.Orientation.X, -PskBonePos.Orientation.Y, PskBonePos.Orientation.Z, PskBonePos.Orientation.W).GetNormalized());

		SkeletalMeshImportData::FJointPos BonePos;
		BonePos.Transform = PskTransform;
		BonePos.Length = PskBonePos.Length;
		BonePos.XSize = PskBonePos.XSize;
		BonePos.YSize = PskBonePos.YSize;
		BonePos.ZSize = PskBonePos.ZSize;

		Bone.BonePos = BonePos;
		SkeletalMeshImportData.RefBonesBinary.Add(Bone);
	}

	for (auto PskInfluence : Psk.Influences)
	{
		SkeletalMeshImportData::FRawBoneInfluence Influence;
		Influence.BoneIndex = PskInfluence.BoneIdx;
		Influence.VertexIndex = PskInfluence.PointIdx;
		Influence.Weight = PskInfluence.Weight;
		SkeletalMeshImportData.Influences.Add(Influence);
	}

	for (auto PskMaterial : Psk.Materials)
	{
		SkeletalMeshImportData::FMaterial Material;
		Material.MaterialImportName = PskMaterial.MaterialName;

		auto MaterialInstance = FActorXUtils::LocalFindOrCreate<UMaterialInstanceConstant>(UMaterialInstanceConstant::StaticClass(), Parent, PskMaterial.MaterialName, Flags);
		Material.Material = MaterialInstance;
		SkeletalMeshImportData.Materials.Add(Material);
	}
	SkeletalMeshImportData.MaxMaterialIndex = SkeletalMeshImportData.Materials.Num()-1;

	SkeletalMeshImportData.bDiffPose = false;
	SkeletalMeshImportData.bHasNormals = Psk.bHasVertexNormals;
	SkeletalMeshImportData.bHasTangents = false;
	SkeletalMeshImportData.bHasVertexColors = true;
	SkeletalMeshImportData.NumTexCoords = 1 + Psk.ExtraUVs.Num(); 
	SkeletalMeshImportData.bUseT0AsRefPose = false;
	
	const auto Skeleton = FActorXUtils::LocalCreate<USkeleton>(USkeleton::StaticClass(), Parent,  Name.ToString().Append("_Skeleton"), Flags);

	FReferenceSkeleton RefSkeleton;
	auto SkeletalDepth = 0;
	ProcessSkeleton(SkeletalMeshImportData, Skeleton, RefSkeleton, SkeletalDepth);

	TArray<FVector3f> LODPoints;
	TArray<SkeletalMeshImportData::FMeshWedge> LODWedges;
	TArray<SkeletalMeshImportData::FMeshFace> LODFaces;
	TArray<SkeletalMeshImportData::FVertInfluence> LODInfluences;
	TArray<int32> LODPointToRawMap;
	SkeletalMeshImportData.CopyLODImportData(LODPoints, LODWedges, LODFaces, LODInfluences, LODPointToRawMap);

	FSkeletalMeshLODModel LODModel;
	LODModel.NumTexCoords = FMath::Max<uint32>(1, SkeletalMeshImportData.NumTexCoords);

	const auto SkeletalMesh = NewObject<USkeletalMesh>(Parent, USkeletalMesh::StaticClass(), Name, Flags);
	SkeletalMesh->PreEditChange(nullptr);
	SkeletalMesh->InvalidateDeriveDataCacheGUID();
	SkeletalMesh->UnregisterAllMorphTarget();

	SkeletalMesh->GetRefBasesInvMatrix().Empty();
	SkeletalMesh->GetMaterials().Empty();
	SkeletalMesh->SetHasVertexColors(true);

	FSkeletalMeshModel* ImportedResource = SkeletalMesh->GetImportedModel();
	auto& SkeletalMeshLODInfos = SkeletalMesh->GetLODInfoArray();
	SkeletalMeshLODInfos.Empty();
	SkeletalMeshLODInfos.Add(FSkeletalMeshLODInfo());
	SkeletalMeshLODInfos[0].ReductionSettings.NumOfTrianglesPercentage = 1.0f;
	SkeletalMeshLODInfos[0].ReductionSettings.NumOfVertPercentage = 1.0f;
	SkeletalMeshLODInfos[0].ReductionSettings.MaxDeviationPercentage = 0.0f;
	SkeletalMeshLODInfos[0].LODHysteresis = 0.02f;

	ImportedResource->LODModels.Empty();
	ImportedResource->LODModels.Add(new FSkeletalMeshLODModel);
	SkeletalMesh->SetRefSkeleton(RefSkeleton);
	SkeletalMesh->CalculateInvRefMatrices();

	SkeletalMesh->SaveLODImportedData(0, SkeletalMeshImportData);
	FSkeletalMeshBuildSettings BuildOptions;
	BuildOptions.bRemoveDegenerates = true;
	BuildOptions.bRecomputeNormals = !Psk.bHasVertexNormals;
	BuildOptions.bRecomputeTangents = true;
	BuildOptions.bUseMikkTSpace = true;
	SkeletalMesh->GetLODInfo(0)->BuildSettings = BuildOptions;
	SkeletalMesh->SetImportedBounds(FBoxSphereBounds(FBoxSphereBounds3f(FBox3f(SkeletalMeshImportData.Points))));

	auto& MeshBuilderModule = IMeshBuilderModule::GetForRunningPlatform();
	const FSkeletalMeshBuildParameters SkeletalMeshBuildParameters(SkeletalMesh, GetTargetPlatformManagerRef().GetRunningTargetPlatform(), 0, false);
	if (!MeshBuilderModule.BuildSkeletalMesh(SkeletalMeshBuildParameters))
	{
		SkeletalMesh->MarkAsGarbage();
		return nullptr;
	}

	for (auto Material : SkeletalMeshImportData.Materials)
	{
		SkeletalMesh->GetMaterials().Add(FSkeletalMaterial(Material.Material.Get()));
	}

	SkeletalMesh->PostEditChange();
	
	SkeletalMesh->SetSkeleton(Skeleton);
	Skeleton->MergeAllBonesToBoneTree(SkeletalMesh);
	
	FAssetRegistryModule::AssetCreated(SkeletalMesh);
	SkeletalMesh->MarkPackageDirty();

	Skeleton->PostEditChange();
	FAssetRegistryModule::AssetCreated(Skeleton);
	Skeleton->MarkPackageDirty();

	return SkeletalMesh;
}

void UPSKFactory::ProcessSkeleton(const FSkeletalMeshImportData& ImportData, const USkeleton* Skeleton, FReferenceSkeleton& OutRefSkeleton, int& OutSkeletalDepth)
{
	const auto RefBonesBinary = ImportData.RefBonesBinary;
	OutRefSkeleton.Empty();
	
	FReferenceSkeletonModifier RefSkeletonModifier(OutRefSkeleton, Skeleton);
	
	for (const auto Bone : RefBonesBinary)
	{
		const FMeshBoneInfo BoneInfo(FName(*Bone.Name), Bone.Name, Bone.ParentIndex);
		RefSkeletonModifier.Add(BoneInfo, FTransform(Bone.BonePos.Transform));
	}

    OutSkeletalDepth = 0;

    TArray<int> SkeletalDepths;
    SkeletalDepths.Empty(ImportData.RefBonesBinary.Num());
    SkeletalDepths.AddZeroed(ImportData.RefBonesBinary.Num());
    for (auto b = 0; b < OutRefSkeleton.GetNum(); b++)
    {
        const auto Parent = OutRefSkeleton.GetParentIndex(b);
        auto Depth  = 1.0f;

        SkeletalDepths[b] = 1.0f;
        if (Parent != INDEX_NONE)
        {
            Depth += SkeletalDepths[Parent];
        }
        if (OutSkeletalDepth < Depth)
        {
            OutSkeletalDepth = Depth;
        }
        SkeletalDepths[b] = Depth;
    }
}
