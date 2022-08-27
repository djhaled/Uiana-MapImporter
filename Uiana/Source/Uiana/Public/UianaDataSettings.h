#pragma once

#include "Engine/EngineTypes.h"
#include "UianaDataSettings.generated.h"
UENUM()
enum WeaponRole
{
	Ascent UMETA(DisplayName = "Ascent"),
	Split UMETA(DisplayName = "Split"),
	Bind UMETA(DisplayName = "Bind"),
	Icebox UMETA(DisplayName = "Icebox"),
	Breeze UMETA(DisplayName = "Breeze"),
	Haven UMETA(DisplayName = "Haven"),
	Fracture UMETA(DisplayName = "Fracture"),
	Range UMETA(DisplayName = "Range"),
	Pearl UMETA(DisplayName = "Pearl"),
	CharacterSelect UMETA(DisplayName = "CharacterSelect"),
	Menu UMETA(DisplayName = "Menu"),
};

UCLASS(config = Engine, defaultconfig, transient)

class UIANA_API UUianaDataSettings : public UObject
{
	GENERATED_BODY()
public:
	UUianaDataSettings(const FObjectInitializer& ObjectInitializer);
	UPROPERTY(config, EditAnywhere, Category = "Settings Folders", meta = (RelativeToGameContentDir, ContentDir))
	FDirectoryPath ExportFolder;
	UPROPERTY(config, EditAnywhere, Category = "Settings Folders", meta = (RelativeToGameContentDir, ContentDir))
	FDirectoryPath PaksFolder;
	UPROPERTY(config, EditAnywhere, Category = "Import Settings")
	TEnumAsByte<WeaponRole> Map;
	UPROPERTY(config, EditAnywhere, Category = "Import Settings")
	bool ImportMeshes;
	UPROPERTY(config, EditAnywhere, Category = "SubLevels")
	bool UseSubLevels;
	UPROPERTY(config, EditAnywhere, Category = "Import Settings")
	bool ImportMaterials;
	UPROPERTY(config, EditAnywhere, Category = "Import Settings")
	bool ImportDecals;
	UPROPERTY(config, EditAnywhere, Category = "Import Settings")
	bool ImportLights;
};
