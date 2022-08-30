#include "UianaDataSettings.h"
#include "Internationalization/Text.h"

UUianaDataSettings::UUianaDataSettings(const FObjectInitializer& ObjectInitializer)
{
	ExportFolder.Path = "C:/Exports";
	PaksFolder.Path = "C:/Riot Games/VALORANT/live/ShooterGame/Content/Paks";
	MapName = "Default";
	AesKey = 0x0;
	GameVersion = GAME_Valorant;
	ImportMeshes = true;
	ImportMaterials = true;
	ImportDecals = false;
	ImportLights = true;
	UseSubLevels = true;
}
