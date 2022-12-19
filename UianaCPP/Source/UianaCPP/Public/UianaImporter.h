#pragma once
#include "UianaCPPDataSettings.h"
#include <BPFL.h>

#include "AssetImporter.h"
#include "Misc/Paths.h"
#include "JsonUtilities.h"
#include "EditorAssetLibrary.h" 
#include "UianaHelpers.h"
#include "Materials/MaterialInstanceConstant.h"
#include "AssetToolsModule.h"
#include "BlueprintImporter.h"
#include "EditorDirectories.h"
#include "EditorLevelLibrary.h"
#include "LevelEditorSubsystem.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "HAL/FileManagerGeneric.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MaterialEditingLibrary.h"
#include "MaterialImporter.h"
#include "Materials/MaterialInstance.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "ObjectEditorUtils.h"
#include "UianaSettings.h"
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
	static TArray<FString> UMaps;
	static UianaSettings Settings;
	static AssetImporter AssetImporterComp;
	static MaterialImporter MaterialImporterComp;
	static BlueprintImporter BlueprintImporterComp;
	
	static FString CreateNewLevel(const FString levelName);
	
	static void ImportUmap(const TArray<TSharedPtr<FJsonValue>> umapData, const FString umapName);
	static void ImportMesh(const TSharedPtr<FJsonObject> obj, const FString umapName, const TMap<FString, AActor*> bpMapping);
	static void ImportDecal(const TSharedPtr<FJsonObject> obj);
	static void ImportLight(const TSharedPtr<FJsonObject> obj);
	
	static void SetBPSettings(const TSharedPtr<FJsonObject> bpProps, UActorComponent* bp);

	static bool IsBlacklisted(const FString itemName);
};
