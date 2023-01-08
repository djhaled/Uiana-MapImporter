#pragma once
#include "UianaCPPDataSettings.h"

#include "FAssetImporter.h"
#include "JsonUtilities.h"
#include "MeshBlueprintImporter.h"
#include "EditorDirectories.h"
#include "HAL/FileManagerGeneric.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MaterialImporter.h"
#include "FDecalLightImporter.h"
#include "UianaSettings.h"
#include "VectorTypes.h"
#include "Engine/AssetManager.h"
#include "Tools/UAssetEditor.h"
#include "UianaImporter.generated.h"

USTRUCT()
struct FUianaExport
{
	GENERATED_BODY();
	UPROPERTY()
	FString version;

	friend FArchive& operator <<(FArchive& Ar, const FUianaExport& ToSerialize)
	{
		FString JsonString;
		FJsonObjectConverter::UStructToJsonObjectString(ToSerialize.StaticStruct(), &ToSerialize, JsonString);
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
	static void Initialize(const FString MapName, UUianaCPPDataSettings* InputSettings);
	static void ImportMap();
private:
	static TArray<FString> UMaps;
	static UianaSettings Settings;
	static FAssetImporter AssetImporterComp;
	static MaterialImporter MaterialImporterComp;
	static MeshBlueprintImporter MeshBlueprintImporterComp;
	static FDecalLightImporter DecalLightImporterComp;
	
	static FString CreateNewLevel(const FString LevelName);
	
	static void ImportUmap(const TArray<TSharedPtr<FJsonValue>> UmapData, const FString UmapName);
	
	static bool IsBlacklisted(const FString ItemName);
};
