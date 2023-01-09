// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UianaCPPDataSettings.h"

/**
 * 
 */
class UianaSettings
{
public:
	const static FString AesKey, TextureFormat;
	constexpr static bool DevForceReexport = false;
	const static TSet<FString> Shaders, BlacklistedObjs;
	FString Name;
	FString ValorantVersion;
	FDirectoryPath ExportFolder;
	FDirectoryPath PaksPath;
	FDirectoryPath FolderPath;
	FDirectoryPath ToolsPath;
	FDirectoryPath AssetsPath;
	FDirectoryPath ExportAssetsPath;
	FDirectoryPath ExportMapsPath;
	FDirectoryPath MaterialsPath;
	FDirectoryPath MaterialsOvrPath;
	FDirectoryPath ObjectsPath;
	FDirectoryPath ScenesPath;
	FDirectoryPath UMapsPath;
	FDirectoryPath UMapJsonPath;
	FDirectoryPath ActorsPath;
	bool ImportMaterials;
	bool ImportMeshes;
	bool UseSubLevels;
	bool ImportDecals;
	bool ImportLights;
	bool ImportBlueprints;
	float LightmapResolutionMultiplier;

	UianaSettings();
	UianaSettings(FString MapName, UUianaCPPDataSettings* Settings);
};
