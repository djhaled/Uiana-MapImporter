#include "UianaCPPDataSettings.h"

#include "Internationalization/Text.h"

UUianaCPPDataSettings::UUianaCPPDataSettings(const FObjectInitializer& ObjectInitializer)
{
	PaksFolder.Path = "C:/Riot Games/VALORANT/live/ShooterGame/Content/Paks";
	ExportFolder.Path = "C:/Exports";
	Map = Ascent;
	ImportMeshes = true;
	ImportMaterials = true;
	ImportDecals = false;
	ImportLights = true;
	UseSubLevels = true;
}

FString UUianaCPPDataSettings::GetExportAssetsFolder()
{
	return FPaths::Combine(ExportFolder, "/export");
}

FString UUianaCPPDataSettings::GetExportMapsFolder()
{
	return FPaths::Combine(ExportFolder, "/maps");
}