#include "UianaDataSettings.h"
#include "Internationalization/Text.h"

UUianaDataSettings::UUianaDataSettings(const FObjectInitializer& ObjectInitializer)
{
	ExportFolder.Path = "C:/Exports";
	PaksFolder.Path = "C:/Riot Games/VALORANT/live/ShooterGame/Content/Paks";
	Map = Ascent;
	GameVersion = GAME_Valorant;
	ImportMeshes = true;
	ImportMaterials = true;
	ImportDecals = false;
	ImportLights = true;
	UseSubLevels = true;
}
