// Fill out your copyright notice in the Description page of Project Settings.


#include "UianaSettings.h"

#include "UianaHelpers.h"

const FString UianaSettings::AesKey = "0x4BE71AF2459CF83899EC9DC2CB60E22AC4B3047E0211034BBABE9D174C069DD6";
const FString UianaSettings::TextureFormat = ".png";
const TSet<FString> UianaSettings::Shaders = {"VALORANT_Base", "VALORANT_Decal", "VALORANT_Emissive",
		"VALORANT_Emissive_Scroll", "VALORANT_Hologram", "VALORANT_Glass", "VALORANT_Blend", "VALORANT_Decal",
		"VALORANT_MRA_Splitter", "VALORANT_Normal_Fix", "VALORANT_Screen"};
const TSet<FString> UianaSettings::BlacklistedObjs = {
	"navmesh",
	"_breakable",
	"_collision",
	"windstreaks_plane",
	"sm_port_snowflakes_boundmesh",
	"M_Pitt_Caustics_Box",
	"box_for_volumes",
	"BombsiteMarker_0_BombsiteA_Glow",
	"BombsiteMarker_0_BombsiteB_Glow",
	"_col",
	"M_Pitt_Lamps_Glow",
	"SM_Pitt_Water_Lid",
	"Bombsite_0_ASiteSide",
	"Bombsite_0_BSiteSide"
	"For_Volumes",
	"Foxtrot_ASite_Plane_DU",
	"Foxtrot_ASite_Side_DU",
	"BombsiteMarker_0_BombsiteA_Glow",
	"BombsiteMarker_0_BombsiteB_Glow",
	"DirtSkirt",
	"Tech_0_RebelSupplyCargoTarpLargeCollision"
};

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
