#include "UianaCPPDataSettings.h"

#include "Internationalization/Text.h"
#include "Misc/FileHelper.h"

UUianaCPPDataSettings::UUianaCPPDataSettings(const FObjectInitializer& ObjectInitializer)
{
	PaksFolder.Path = TEXT("C:/Riot Games/VALORANT/live/ShooterGame/Content/Paks");
	ExportFolder.Path = TEXT("C:/Exports");
	Map = Ascent;
	ImportMeshes = true;
	ImportMaterials = true;
	ImportDecals = false;
	ImportLights = true;
	UseSubLevels = true;
	LightmapResolutionMultiplier = 1;
	ValorantVersion = GetValorantInstall();
}

FString UUianaCPPDataSettings::GetValorantInstall()
{
	if(FPaths::FileExists(ValorantMetadataPath))
	{
		FString metadata;
		FFileHelper::LoadFileToString(metadata, *ValorantMetadataPath);
		FString firstLine, lineSplit, version, temp;
		metadata.Split(TEXT("\n"), &firstLine, &temp);
		firstLine.Split(TEXT("/"), &temp, &lineSplit, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		lineSplit.Split(TEXT("."), &version, &temp);
		return version;
	}
	UE_LOG(LogScript, Warning, TEXT("Failed to get Valorant version, setting it to 0"));
	return "0";
}
