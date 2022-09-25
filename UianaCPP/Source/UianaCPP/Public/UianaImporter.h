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
#include "UianaMaterialStructs.h"
#include "UianaImporter.generated.h"

USTRUCT()
struct FUMap3DProperty
{
	GENERATED_BODY();
	UPROPERTY()
	float X;
	UPROPERTY()
	float Y;
	UPROPERTY()
	float Z;
};

USTRUCT()
struct FUMapColorProperty
{
	GENERATED_BODY();
	UPROPERTY()
	int8 B;
	UPROPERTY()
	int8 G;
	UPROPERTY()
	int8 R;
	UPROPERTY()
	int8 A;
	UPROPERTY()
	FString Hex;
};

USTRUCT()
struct FUMapComponentProperties
{
	GENERATED_BODY();
	TOptional<FUMapObjectProperty> StaticMesh;
	TOptional<TArray<FUMapObjectProperty>> OverrideMaterials;
	// UPROPERTY()
	// FUMapObjectProperty Brush;
	// UPROPERTY()
	// FUMapObjectProperty BrushBodySetup;
	TOptional<FUMapObjectProperty> DecalMaterial;
	// UPROPERTY()
	// FUMap3DProperty DecalSize;
	// UPROPERTY()
	// FUMap3DProperty RelativeLocation;
	// UPROPERTY()
	// FUMap3DProperty RelativeRotation;
	// UPROPERTY()
	// FUMap3DProperty RelativeScale3D;
	// UPROPERTY()
	// FString DetailMode;
	// UPROPERTY()
	// bool bOverrideColor;
	TOptional<bool> bVisible;
	// UPROPERTY()
	// int VisibilityId;
};

USTRUCT()
struct FUMapComponent
{
	GENERATED_USTRUCT_BODY();
	UPROPERTY()
	FString Type;
	UPROPERTY()
	FString Name;
	UPROPERTY();
	FString Outer;
	UPROPERTY()
	FUMapComponentProperties Properties;

	friend FArchive& operator <<(FArchive& Ar, FUMapComponent& toSerialize)
	{
		FString JsonString;
		FJsonObjectConverter::UStructToJsonObjectString(toSerialize.StaticStruct(), &toSerialize, JsonString);
		Ar << JsonString;
		return Ar;
	}
};

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
	UUianaImporter(FString MapName, UUianaCPPDataSettings Settings);
	static void ImportMap(UUianaCPPDataSettings Settings);
private:
	inline const static FString AesKey = "0x4BE71AF2459CF83899EC9DC2CB60E22AC4B3047E0211034BBABE9D174C069DD6";
	inline const static FString TextureFormat = ".png";
	inline constexpr static bool DevForceReexport = false;
	inline const static FString Shaders[] = {"VALORANT_Base", "VALORANT_Decal", "VALORANT_Emissive",
		"VALORANT_Emissive_Scroll", "VALORANT_Hologram", "VALORANT_Glass", "VALORANT_Blend", "VALORANT_Decal",
		"VALORANT_MRA_Splitter", "VALORANT_Normal_Fix", "VALORANT_Screen"};
	static FString Name;
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
	static FDirectoryPath ActorsPath;

	static bool NeedExport(UUianaCPPDataSettings const &Settings);
	static void ExtractAssets(TArray<FString> umapPaths);
	static void CUE4Extract(FDirectoryPath ExportDir, FString AssetList = "");
	static void UModelExtract();
	static FString CreateNewLevel();
	static void GetTexturePaths(const TArray<FString> matPaths, TArray<FString> &texturePaths);
	static void CreateMaterial(const TArray<FString> matPaths, TMap<FString, UMaterialInstance*> loadableMaterials);
	static void SetMaterial(FUianaMaterialJson matData, UMaterialInstanceConstant* mat);
	static void SetTextures(FUianaMaterialJson matData, UMaterialInstanceConstant* mat);
	static void SetMaterialSettings(FUianaMaterialProperties matProps, UMaterialInstanceConstant* mat);
	static void UUianaImporter::GetObjects(TArray<FString> &actorPaths, TArray<FString> &objPaths,
		TArray<FString> &matPaths, const TArray<TSharedPtr<FJsonValue>> &jsonArr);
};
