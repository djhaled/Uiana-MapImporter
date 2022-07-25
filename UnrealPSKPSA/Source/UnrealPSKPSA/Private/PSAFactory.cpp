// Fill out your copyright notice in the Description page of Project Settings.


#include "PSAFactory.h"

#include "EditorAssetLibrary.h"
#include "PSAReader.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Misc/ScopedSlowTask.h"
#include "Rendering/SkeletalMeshLODImporterData.h"

UObject* UPSAFactory::Import(const FString Filename, UObject* Parent, const FName Name, const EObjectFlags Flags) const
{
	auto Psa = PSAReader(Filename);
	if (!Psa.Read()) return nullptr;
	
	auto AnimSequence = NewObject<UAnimSequence>(Parent, UAnimSequence::StaticClass(), Name, Flags);
	
	auto Skeleton = CastChecked<USkeleton>(UEditorAssetLibrary::LoadAsset("/Game/M_MED_Heartache_Skeleton.M_MED_Heartache_Skeleton"));
	AnimSequence->SetSkeleton(Skeleton);
	auto SkeletalMesh = CastChecked<USkeletalMesh>(UEditorAssetLibrary::LoadAsset("/Game/M_MED_Heartache_LOD0.M_MED_Heartache"));
	AnimSequence->CreateAnimation(SkeletalMesh);

	auto MeshBones = Skeleton->GetReferenceSkeleton().GetRawRefBoneInfo();

	auto& AnimController = AnimSequence->GetController();

	auto Info = Psa.AnimInfo;
	AnimController.SetFrameRate(FFrameRate(Info.AnimRate, 1));
	AnimController.SetPlayLength(Info.NumRawFrames/Info.AnimRate);
	
	FScopedSlowTask ImportTask(Psa.Bones.Num(), FText::FromString("Importing Anim"));
	ImportTask.MakeDialog(false);
	for (auto BoneIndex = 0; BoneIndex < Psa.Bones.Num(); BoneIndex++)
	{
		auto Bone = Psa.Bones[BoneIndex];
		auto BoneName = FName(Bone.Name);
		ImportTask.DefaultMessage = FText::FromString(FString::Printf(TEXT("Bone %s: %d/%d"), *BoneName.ToString(), BoneIndex+1, Psa.Bones.Num()));
		ImportTask.EnterProgressFrame();
		
		TArray<FVector3f> PositionalKeys;
		TArray<FQuat4f> RotationalKeys;
		TArray<FVector3f> ScaleKeys;
		for (auto Frame = 0; Frame < Info.NumRawFrames; Frame++)
		{
			auto KeyIndex = BoneIndex + Frame * Psa.Bones.Num();
			auto AnimKey = Psa.AnimKeys[KeyIndex];

			PositionalKeys.Add(FVector3f(AnimKey.Position.X, -AnimKey.Position.Y, AnimKey.Position.Z));
			RotationalKeys.Add(FQuat4f(AnimKey.Orientation.X, -AnimKey.Orientation.Y, AnimKey.Orientation.Z, AnimKey.Orientation.W).GetNormalized());
			ScaleKeys.Add(Psa.bHasScaleKeys ? Psa.ScaleKeys[KeyIndex].ScaleVector : FVector3f::OneVector);
		}

		AnimController.AddBoneTrack(BoneName);
		AnimController.SetBoneTrackKeys(BoneName, PositionalKeys, RotationalKeys, ScaleKeys);
	}
	AnimController.RemoveBoneTracksMissingFromSkeleton(Skeleton);

	AnimSequence->Modify(true);
	AnimSequence->PostEditChange();
	FAssetRegistryModule::AssetCreated(AnimSequence);
	AnimSequence->MarkPackageDirty();

	for (TObjectIterator<USkeletalMeshComponent> Iter; Iter; ++Iter)
	{
		FComponentReregisterContext ReregisterContext(*Iter);
	}

	return AnimSequence;
}
