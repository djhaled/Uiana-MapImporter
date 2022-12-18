// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UianaCPPDataSettings.h"
#include "UObject/Object.h"
#include "UianaSettings.generated.h"

/**
 * 
 */
UCLASS()
class UIANACPP_API UUianaSettings : public UObject
{
	GENERATED_BODY()
public:
	inline const static FString AesKey = "0x4BE71AF2459CF83899EC9DC2CB60E22AC4B3047E0211034BBABE9D174C069DD6";
	inline const static FString TextureFormat = ".png";
	inline constexpr static bool DevForceReexport = false;
	inline const static FString Shaders[] = {"VALORANT_Base", "VALORANT_Decal", "VALORANT_Emissive",
		"VALORANT_Emissive_Scroll", "VALORANT_Hologram", "VALORANT_Glass", "VALORANT_Blend", "VALORANT_Decal",
		"VALORANT_MRA_Splitter", "VALORANT_Normal_Fix", "VALORANT_Screen"};
	inline static TSet<FString> BlacklistedObjs = {
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
	FString Name;
	FString ValorantVersion;
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
	UPROPERTY()
	UUianaCPPDataSettings* InputSettings;

	UUianaSettings();
	void Initialize(FString MapName, UUianaCPPDataSettings* Settings);
};
