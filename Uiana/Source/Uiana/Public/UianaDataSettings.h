#pragma once

#include "Engine/EngineTypes.h"
#include "UianaDataSettings.generated.h"
UENUM()
enum WeaponRole
{
	Ascent     UMETA(DisplayName = "Ascent"),
	Split      UMETA(DisplayName = "Split"),
	Bind   UMETA(DisplayName = "Bind"),
	Icebox     UMETA(DisplayName = "Icebox"),
	Breeze      UMETA(DisplayName = "Breeze"),
	Haven   UMETA(DisplayName = "Haven"),
	Fracture     UMETA(DisplayName = "Fracture"),
	Range      UMETA(DisplayName = "Range"),
	Pearl   UMETA(DisplayName = "Pearl"),
	CharacterSelect   UMETA(DisplayName = "CharacterSelect"),
	Menu   UMETA(DisplayName = "Menu"),
};

UENUM()
enum UE4Version
{
    GAME_UE4_0,
    GAME_UE4_1 UMETA(DisplayName = "GAME_UE4_1"),
    GAME_UE4_2 UMETA(DisplayName = "GAME_UE4_2"),
    GAME_UE4_3 UMETA(DisplayName = "GAME_UE4_3"),
    GAME_UE4_4 UMETA(DisplayName = "GAME_UE4_4"),
    GAME_UE4_5 UMETA(DisplayName = "GAME_UE4_5"),
    GAME_UE4_6 UMETA(DisplayName = "GAME_UE4_6"),
    GAME_UE4_7 UMETA(DisplayName = "GAME_UE4_7"),
    GAME_UE4_8 UMETA(DisplayName = "GAME_UE4_8"),
    GAME_UE4_9 UMETA(DisplayName = "GAME_UE4_9"),
    GAME_UE4_10 UMETA(DisplayName = "GAME_UE4_10"),
    GAME_SeaOfThieves UMETA(DisplayName = "GAME_SeaOfThieves"),
    GAME_UE4_11 UMETA(DisplayName = "GAME_UE4_11"),
    GAME_UE4_12 UMETA(DisplayName = "GAME_UE4_12"),
    GAME_UE4_13 UMETA(DisplayName = "GAME_UE4_13"),
    GAME_StateOfDecay2 UMETA(DisplayName = "GAME_StateOfDecay2"),
    GAME_UE4_14 UMETA(DisplayName = "GAME_UE4_14"),
    GAME_UE4_15 UMETA(DisplayName = "GAME_UE4_15"),
    GAME_UE4_16 UMETA(DisplayName = "GAME_UE4_16"),
    GAME_PlayerUnknownsBattlegrounds UMETA(DisplayName = "GAME_PlayerUnknownsBattlegrounds"),
    GAME_UE4_17 UMETA(DisplayName = "GAME_UE4_17"),
    GAME_UE4_18 UMETA(DisplayName = "GAME_UE4_18"),
    GAME_UE4_19 UMETA(DisplayName = "GAME_UE4_19"),
    GAME_UE4_20 UMETA(DisplayName = "GAME_UE4_20"),
    GAME_Borderlands3 UMETA(DisplayName = "GAME_Borderlands3"),
    GAME_UE4_21 UMETA(DisplayName = "GAME_UE4_21"),
    GAME_StarWarsJediFallenOrder UMETA(DisplayName = "GAME_StarWarsJediFallenOrder"),
    GAME_UE4_22 UMETA(DisplayName = "GAME_UE4_22"),
    GAME_UE4_23 UMETA(DisplayName = "GAME_UE4_23"),
    GAME_UE4_24 UMETA(DisplayName = "GAME_UE4_24"),
    GAME_UE4_25 UMETA(DisplayName = "GAME_UE4_25"),
    GAME_RogueCompany  UMETA(DisplayName = "GAME_RogueCompany"),
    GAME_KenaBridgeofSpirits  UMETA(DisplayName = "GAME_KenaBridgeofSpirits"),
    GAME_UE4_25_Plus  UMETA(DisplayName = "GAME_UE4_25_Plus"),
    GAME_UE4_26 UMETA(DisplayName = "GAME_UE4_26"),
    GAME_GTATheTrilogyDefinitiveEdition  UMETA(DisplayName = "GAME_GTATheTrilogyDefinitiveEdition"),
    GAME_ReadyOrNot  UMETA(DisplayName = "GAME_ReadyOrNot"),
    GAME_Valorant  UMETA(DisplayName = "GAME_Valorant"),
    GAME_UE4_27 UMETA(DisplayName = "GAME_UE4_27"),
    GAME_Splitgate  UMETA(DisplayName = "GAME_Splitgate"),
    GAME_UE4_LATEST  UMETA(DisplayName = "GAME_UE4_LATEST"),
    GAME_UE5_0  UMETA(DisplayName = "GAME_UE5_0"),
    GAME_UE5_1  UMETA(DisplayName = "GAME_UE5_1"),
    GAME_UE5_LATEST  UMETA(DisplayName = "GAME_UE5_LATEST")
};
UCLASS(config = Engine, defaultconfig,transient)

class UIANA_API UUianaDataSettings : public UObject
{
	GENERATED_BODY()
public:
	UUianaDataSettings(const FObjectInitializer& ObjectInitializer);
	UPROPERTY(config, EditAnywhere, Category = "Settings Folders", meta = (RelativeToGameContentDir, ContentDir))
		FDirectoryPath ExportFolder;
	UPROPERTY(config, EditAnywhere, Category = "Settings Folders", meta = (RelativeToGameContentDir, ContentDir))
		FDirectoryPath PaksFolder;
	//UPROPERTY(config, EditAnywhere, Category = "Import Settings")
		//TEnumAsByte<WeaponRole> Map;
    /** Your map name thats in uiana/content/python/assets/umaps.json */
    UPROPERTY(config, EditAnywhere, Category = "Game Settings")
        FString MapName;
    UPROPERTY(config, EditAnywhere, Category = "Game Settings")
        TEnumAsByte<UE4Version> GameVersion;
    UPROPERTY(config, EditAnywhere, Category = "Game Settings")
        FString AesKey;
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