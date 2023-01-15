#pragma once

#include "Engine/EngineTypes.h"
#include "Interfaces/IPluginManager.h"
#include "UianaCPPDataSettings.generated.h"
UENUM()
enum WeaponRoleCPP
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

class UIANACPP_API UUianaCPPDataSettings : public UObject
{
	GENERATED_BODY()
public:
	// Properties visible to end-user
	UPROPERTY(config, EditAnywhere, Category = "Settings Folders", meta = (RelativeToGameContentDir, ContentDir))
	FDirectoryPath ExportFolder;
	UPROPERTY(config, EditAnywhere, Category = "Settings Folders", meta = (RelativeToGameContentDir, ContentDir))
	FDirectoryPath PaksFolder;
	UPROPERTY(config, EditAnywhere, Category = "Import Settings")
	TEnumAsByte<WeaponRoleCPP> Map;
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
	UPROPERTY(config, EditAnywhere, Category = "Import Settings")
	bool ImportBlueprints;
	UPROPERTY(config, EditAnywhere, Category = "Import Settings", meta=(ClampMin=0.01, ClampMax=5))
	float LightmapResolutionMultiplier;
	
	UFUNCTION()
	FString GetValorantInstall();
	UPROPERTY()
	FString ValorantVersion;
	
	// Functions
	UUianaCPPDataSettings(const FObjectInitializer& ObjectInitializer);

private:
	const FString ValorantMetadataPath = TEXT("C:\\ProgramData\\Riot Games\\Metadata\\valorant.live\\valorant.live.ok");
};
