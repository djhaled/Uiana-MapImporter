// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UianaHelpers.h"
#include "UianaSettings.h"

/**
 * Handles extracting assets for Uiana and providing paths to umaps
 */
class AssetImporter
{
public:
	AssetImporter();
	AssetImporter(const UUianaSettings* Settings);
	TArray<FString> GetExtractedUmaps();

private:
	const UUianaSettings* Settings;
	bool NeedExport();
	TArray<FString> ExtractAssets();
	void GetObjects(TArray<FString> &actorPaths, TArray<FString> &objPaths, TArray<FString> &matPaths, const TArray<TSharedPtr<FJsonValue>> &jsonArr);
	void CUE4Extract(const FDirectoryPath ExportDir);
	void CUE4Extract(const FDirectoryPath ExportDir, const FString AssetList);
	void UModelExtract();
};
