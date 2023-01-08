// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UianaHelpers.h"
#include "UianaSettings.h"

/**
 * Handles extracting assets for Uiana and providing paths to umaps
 */
class FAssetImporter
{
public:
	FAssetImporter();
	explicit FAssetImporter(const UianaSettings* Settings);
	TArray<FString> GetExtractedUmaps();

private:
	const UianaSettings* Settings;
	bool NeedExport();
	TArray<FString> ExtractAssets();
	void GetObjects(TArray<FString> &ActorPaths, TArray<FString> &ObjPaths, TArray<FString> &MatPaths, const TArray<TSharedPtr<FJsonValue>> &JsonArr) const;
	void Cue4Extract(const FDirectoryPath ExportDir) const;
	void Cue4Extract(const FDirectoryPath ExportDir, const FString AssetList) const;
	void UModelExtract() const;
};
