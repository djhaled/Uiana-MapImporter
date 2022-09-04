#pragma once
#include "UianaCPPDataSettings.h"
#include <BPFL.h>
#include "Misc/Paths.h"
#include "JsonUtilities.h"
#include "UianaHelpers.h"

USTRUCT()
struct FUMapObjectProperty
{
	GENERATED_BODY();
	UPROPERTY()
	FString ObjectName;
	UPROPERTY()
	FString ObjectPath;
};

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
	// UPROPERTY()
	// FUMapObjectProperty Brush;
	// UPROPERTY()
	// FUMapObjectProperty BrushBodySetup;
	// UPROPERTY()
	// FUMapObjectProperty DecalMaterial;
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

UCLASS()
class UIANACPP_API UUianaImporter : public UObject
{
public:
	UUianaImporter(FString MapName, UUianaCPPDataSettings Settings);
	static void ImportMap(UUianaCPPDataSettings Settings);
private:
	inline const static FString AesKey = "0x4BE71AF2459CF83899EC9DC2CB60E22AC4B3047E0211034BBABE9D174C069DD6";
	inline const static FString TextureFormat = ".png";
	static FString Name;
	static TArray<FString> UMaps;
	static FDirectoryPath PaksPath, FolderPath, ToolsPath, AssetsPath, ExportAssetsPath, ExportMapsPath, 
		MaterialsPath, MaterialsOvrPath, ObjectsPath, ScenesPath, UMapsPath;

	static void ExtractAssets(TArray<FString> umapPaths);
	static void CUE4Extract(FDirectoryPath ExportDir, FString AssetList = "");
	static void UModelExtract();
};
