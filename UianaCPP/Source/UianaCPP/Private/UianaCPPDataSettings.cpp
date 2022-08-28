#include "UianaCPPDataSettings.h"
#include "Internationalization/Text.h"

UUianaCPPDataSettings::UUianaCPPDataSettings(const FObjectInitializer& ObjectInitializer)
{
	ExportFolder.Path = "C:/Exports";
	PaksFolder.Path = "C:/Riot Games/VALORANT/live/ShooterGame/Content/Paks";
	Map = Ascent;
	ImportMeshes = true;
	ImportMaterials = true;
	ImportDecals = false;
	ImportLights = true;
	UseSubLevels = true;
}
