#include "UianaDataSettings.h"
#include "Internationalization/Text.h"

UUianaDataSettings::UUianaDataSettings(const FObjectInitializer& ObjectInitializer)
{
	ExportFolder.Path = "C:/Exports";
	PaksFolder.Path = "C:/Riot Games/VALORANT/live/ShooterGame/Content/Paks";
	Map = Split;
	ImportMeshes = true;
	ImportMaterials = true;
	ImportDecals = true;
	ImportLights = true;
	ImportBlueprints = true;
	UseSubLevels = true;
	LightmapResolutionMultiplier = 1;
}
