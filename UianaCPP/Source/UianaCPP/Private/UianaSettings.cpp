// Fill out your copyright notice in the Description page of Project Settings.


#include "UianaSettings.h"

#include "UianaHelpers.h"

UianaSettings::UianaSettings()
{
	PaksPath.Path = "C:/Riot Games/VALORANT/live/ShooterGame/Content/Paks";
	Name = "Ascent";
	ImportMeshes = true;
	ImportMaterials = true;
	ImportDecals = false;
	ImportLights = true;
	UseSubLevels = true;
	LightmapResolutionMultiplier = 1;
	ValorantVersion = "1.0";
}

UianaSettings::UianaSettings(FString MapName, UUianaCPPDataSettings* Settings)
{
	FString RelativeContentDir = IPluginManager::Get().FindPlugin(TEXT("UianaCPP"))->GetContentDir();
	FString ContentDir = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*RelativeContentDir);
	PaksPath = Settings->PaksFolder;
	ValorantVersion = Settings->ValorantVersion;
	ImportMaterials = Settings->ImportMaterials;
	ImportBlueprints = Settings->ImportBlueprints;
	ImportDecals = Settings->ImportDecals;
	ImportLights = Settings->ImportLights;
	ImportMeshes = Settings->ImportMeshes;
	UseSubLevels = Settings->UseSubLevels;
	LightmapResolutionMultiplier = Settings->LightmapResolutionMultiplier;
	
	Name = MapName;
	// Create content directories
	UianaHelpers::CreateFolder(ToolsPath, ContentDir, "tools"); // tools_path
	UianaHelpers::CreateFolder(AssetsPath, ContentDir, "assets"); // importer_assets_path
	UianaHelpers::CreateFolder(ExportAssetsPath, Settings->ExportFolder.Path, "export"); // assets_path
	UianaHelpers::CreateFolder(ExportMapsPath, Settings->ExportFolder.Path, "maps"); // maps_path
	UianaHelpers::CreateFolder(FolderPath, ExportMapsPath.Path, "" + MapName);
	UianaHelpers::CreateFolder(MaterialsPath, FolderPath.Path, "materials");
	UianaHelpers::CreateFolder(MaterialsOvrPath, FolderPath.Path, "materials_ovr");
	UianaHelpers::CreateFolder(ObjectsPath, FolderPath.Path, "objects");
	UianaHelpers::CreateFolder(ScenesPath, FolderPath.Path, "scenes");
	UianaHelpers::CreateFolder(UMapsPath, FolderPath.Path, "umaps");
	UianaHelpers::CreateFolder(ActorsPath, FolderPath.Path, "actors");
}
