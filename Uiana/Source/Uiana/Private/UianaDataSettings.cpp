#include "UianaDataSettings.h"
#include "Internationalization/Text.h"

UUianaDataSettings::UUianaDataSettings(const FObjectInitializer& ObjectInitializer)
{
	ExportFolder.Path = "C:/Exports";
	PaksFolder.Path = "C:/Riot Games/VALORANT/live/ShooterGame/Content/Paks";
	AesKey = FString("0x4BE71AF2459CF83899EC9DC2CB60E22AC4B3047E0211034BBABE9D174C069DD6");
	Map = Ascent;
	ImportMisc = true;
	ImportMeshes = true;
	ImportMaterials = true;
	ImportDecals = false;
	ImportLights = false;
}
