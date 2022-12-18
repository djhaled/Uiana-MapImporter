// Fill out your copyright notice in the Description page of Project Settings.


#include "UianaSettings.h"

#include "UianaHelpers.h"

UUianaSettings::UUianaSettings()
{
	
}


void UUianaSettings::Initialize(FString MapName, UUianaCPPDataSettings* Settings)
{
	InputSettings = Settings;
	FString RelativeContentDir = IPluginManager::Get().FindPlugin(TEXT("UianaCPP"))->GetContentDir();
	FString ContentDir = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*RelativeContentDir);
	PaksPath = Settings->PaksFolder;
	ValorantVersion = Settings->ValorantVersion;
	
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
