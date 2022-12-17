#pragma once
#include "UianaCPPDataSettings.h"
#include <BPFL.h>
#include "Misc/Paths.h"
#include "JsonUtilities.h"
#include "EditorAssetLibrary.h" 
#include "UianaHelpers.h"
#include "Materials/MaterialInstanceConstant.h"
#include "AssetToolsModule.h"
#include "EditorDirectories.h"
#include "EditorLevelLibrary.h"
#include "LevelEditorSubsystem.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "HAL/FileManagerGeneric.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MaterialEditingLibrary.h"
#include "Materials/MaterialInstance.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "ObjectEditorUtils.h"
#include "VectorTypes.h"
#include "Components/DecalComponent.h"
#include "Engine/AssetManager.h"
#include "Tools/UAssetEditor.h"
#include "Materials/MaterialInstanceBasePropertyOverrides.h"
#include "Engine/TextureCube.h"
#include "UianaImporter.generated.h"

USTRUCT()
struct FUianaExport
{
	GENERATED_BODY();
	UPROPERTY()
	FString version;

	friend FArchive& operator <<(FArchive& Ar, FUianaExport& toSerialize)
	{
		FString JsonString;
		FJsonObjectConverter::UStructToJsonObjectString(toSerialize.StaticStruct(), &toSerialize, JsonString);
		Ar << JsonString;
		return Ar;
	}
};

UCLASS()
class UIANACPP_API UUianaImporter : public UObject
{
	GENERATED_BODY()
public:
	UUianaImporter();
	static void Initialize(FString MapName, UUianaCPPDataSettings* Settings);
	static void ImportMap();
private:
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
	static UUianaCPPDataSettings* Settings;
	static FString Name;
	static FString ValorantVersion;
	static TArray<FString> UMaps;
	static FDirectoryPath PaksPath;
	static FDirectoryPath FolderPath;
	static FDirectoryPath ToolsPath;
	static FDirectoryPath AssetsPath;
	static FDirectoryPath ExportAssetsPath;
	static FDirectoryPath ExportMapsPath;
	static FDirectoryPath MaterialsPath;
	static FDirectoryPath MaterialsOvrPath;
	static FDirectoryPath ObjectsPath;
	static FDirectoryPath ScenesPath;
	static FDirectoryPath UMapsPath;
	static FDirectoryPath UMapJsonPath;
	static FDirectoryPath ActorsPath;

	static bool NeedExport();
	static void ExtractAssets(TArray<FString> &umapPaths);
	static void CUE4Extract(FDirectoryPath ExportDir);
	static void CUE4Extract(FDirectoryPath ExportDir, FString AssetList);
	static void UModelExtract();
	static FString CreateNewLevel(const FString levelName);
	static void GetTexturePaths(const TArray<FString> matPaths, TArray<FString> &texturePaths);
	static void CreateMaterials(const TArray<FString> matPaths);
	static void CreateBlueprints(const TArray<FString> bpPaths);
	static void SetMaterial(const TSharedPtr<FJsonObject> matData, UMaterialInstanceConstant* mat);
	static void SetTextures(const TSharedPtr<FJsonObject> matData, UMaterialInstanceConstant* mat);
	static void SetMaterialSettings(const TSharedPtr<FJsonObject> matProps, UMaterialInstanceConstant* mat);
	static FMaterialInstanceBasePropertyOverrides SetBasePropertyOverrides(const TSharedPtr<FJsonObject> matProps);
	static void GetObjects(TArray<FString> &actorPaths, TArray<FString> &objPaths,
		TArray<FString> &matPaths, const TArray<TSharedPtr<FJsonValue>> &jsonArr);
	
	static void ImportUmap(const TArray<TSharedPtr<FJsonValue>> umapData, const FString umapName);
	static void ImportBlueprint(const TSharedPtr<FJsonObject> obj, TMap<FString, AActor*> &bpMapping);
	static void ImportMesh(const TSharedPtr<FJsonObject> obj, const FString umapName, const TMap<FString, AActor*> bpMapping);
	static void ImportDecal(const TSharedPtr<FJsonObject> obj);
	static void ImportLight(const TSharedPtr<FJsonObject> obj);

	static TArray<USCS_Node*> GetLocalBPChildren(TArray<TSharedPtr<FJsonValue>> childNodes, TArray<TSharedPtr<FJsonValue>> bpData, UBlueprint* bpActor);
	static void SetBPSettings(const TSharedPtr<FJsonObject> bpProps, UActorComponent* bp);
	static void SetObjSettings(const TSharedPtr<FJsonObject> bpProps, UObject* mainObj);
	static void FixActorBP(const TSharedPtr<FJsonObject> bpData, const TMap<FString, AActor*> bpMapping);

	static bool IsBlacklisted(const FString itemName);
	static TArray<UMaterialInterface*> CreateOverrideMaterials(const TSharedPtr<FJsonObject> obj);
};
