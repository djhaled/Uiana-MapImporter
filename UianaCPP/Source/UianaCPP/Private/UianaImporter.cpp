#include "UianaImporter.h"

UUianaImporter::UUianaImporter(FString MapName, UUianaCPPDataSettings Settings)
{
	FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("Uiana"))->GetContentDir();
	ToolsPath.Path = FPaths::Combine(ContentDir, "/tools");
	AssetsPath.Path = FPaths::Combine(ContentDir, "/assets");
	// TODO: Cannot initialize UianaMap. Consider getting rid of it entirely and having everything in the Importer for now.
	Map(MapName, Settings);
}

void UUianaImporter::ImportMap(UUianaCPPDataSettings Settings)
{
	UBPFL::ChangeProjectSettings();
}

void UianaImporter::GetMapAssets(UUianaCPPDataSettings Settings)
{
	if (Settings)
}