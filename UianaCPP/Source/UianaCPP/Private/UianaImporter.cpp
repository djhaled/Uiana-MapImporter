#include "UianaImporter.h"

#include "HAL/FileManagerGeneric.h"

UUianaImporter::UUianaImporter(FString MapName, UUianaCPPDataSettings Settings)
{
	Name = MapName;

	// Set paths
	FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("Uiana"))->GetContentDir();
	ToolsPath.Path = FPaths::Combine(ContentDir, "/tools");
	AssetsPath.Path = FPaths::Combine(ContentDir, "/assets");
	ExportAssetsPath.Path = FPaths::Combine(Settings.ExportFolder, "/export");
	ExportMapsPath.Path = FPaths::Combine(Settings.ExportFolder, "/maps");
	PaksPath = Settings.PaksFolder;
	
	// Create content directories
	UianaHelpers::CreateFolder(FolderPath, ExportMapsPath.Path, "/" + MapName);
	UianaHelpers::CreateFolder(MaterialsPath, FolderPath.Path, "/materials");
	UianaHelpers::CreateFolder(MaterialsOvrPath, FolderPath.Path, "/materials_ovr");
	UianaHelpers::CreateFolder(ObjectsPath, FolderPath.Path, "/objects");
	UianaHelpers::CreateFolder(ScenesPath, FolderPath.Path, "/scenes");
	UianaHelpers::CreateFolder(UMapsPath, FolderPath.Path, "/umaps");
	
	// Open umaps JSON file and read the UMaps to store
	FString UMapJSON;
	FFileHelper::LoadFileToString(UMapJSON, *FPaths::Combine(UMapsPath.Path, "/umaps.json"));
	TSharedPtr<FJsonObject> JsonParsed = UianaHelpers::ParseJson(UMapJSON);
	if (!JsonParsed->TryGetStringArrayField(MapName, UMaps))
	{
		UE_LOG(LogTemp, Error, TEXT("UIANA: Failed to deserialize umaps for %s"), *MapName);
	}
}

void UUianaImporter::ImportMap(UUianaCPPDataSettings Settings)
{
	UBPFL::ChangeProjectSettings();
	
	TArray<FString> umapPaths;
	FFileManagerGeneric::Get().FindFiles(umapPaths, *(UMapsPath.Path), TEXT(".json"));
	if (FPaths::FileExists(FPaths::Combine(FolderPath, "/exported.yo")))
	{
		// Use umapPaths as if get_map_assets() finished
	}
	ExtractAssets();
}

void UUianaImporter::ExtractAssets(TArray<FString> umapPaths)
{
	CUE4Extract(UMapsPath);
	UModelExtract();
	for (FString umapPath : umapPaths)
	{
		FString umapStr;
		FFileHelper::LoadFileToString(umapStr, *umapPath);
		TArray<FUMapComponent> umapRaw;
		TArray<FUMapComponent> umapFiltered = {};
		FJsonObjectConverter::JsonArrayStringToUStruct<FUMapComponent>(umapStr, &umapRaw, 0, 0);

		// Filter umap
		const TArray<FString> decalTypes = {"decalcomponent"};
		const TArray<FString> meshTypes = {"staticmesh", "staticmeshcomponent", "instancedstaticmeshcomponent", "hierarchicalinstancedstaticmeshcomponent"};
		const TArray<FString> genTypes = {"pointlightcomponent","postprocessvolume" ,"culldistancevolume","scenecomponent","lightmasscharacterindirectdetailvolume","brushcomponent","precomputedvisibilityvolume","rectlightcomponent", "spotlightcomponent", "skylightcomponent",  "scenecapturecomponentcube","lightmassimportancevolume","billboardcomponent", "directionallightcomponent", "exponentialheightfogcomponent", "lightmassportalcomponent", "spherereflectioncapturecomponent"};
		for (FUMapComponent component : umapRaw)
		{
			FString typeLower = component.Type.ToLower();
			if (meshTypes.Contains(typeLower) && component.Properties.StaticMesh.IsSet() && !component.Properties.bVisible.IsSet())
			{
				umapFiltered.Add(component);
			}
			if (genTypes.Contains(typeLower) || decalTypes.Contains(typeLower))
			{
				umapFiltered.Add(component);
			}
		}
		
		// Save cleaned-up JSON
		FBufferArchive SaveData;
		SaveData << umapFiltered;
		FFileHelper::SaveArrayToFile(SaveData, *umapPath);
		SaveData.FlushCache();
		SaveData.Empty();

		// TODO: Get objects
	}
}

void UUianaImporter::CUE4Extract(FDirectoryPath ExportDir, FString AssetList)
{
	TArray<FStringFormatArg> args = {
		FPaths::Combine(ToolsPath, "/cue4extractor.exe"),
		PaksPath.Path,
		AesKey,
		ExportDir.Path,
		Name,
		AssetList,
		UMapsPath.Path
	};
	FString ConsoleCommand = FString::Format(TEXT("{0} --game-directory {1} --aes-key {2} --export-directory {3} --map-name {4} --file-list {5} --game-umaps {6}"), args);
	GEngine->Exec(nullptr, *ConsoleCommand);
}

void UUianaImporter::UModelExtract()
{
	if (FPaths::FileExists(FPaths::Combine(ExportAssetsPath, "/exported.yo")))
		return;
	TArray<FStringFormatArg> args = {
		FPaths::Combine(ToolsPath, "/umodel.exe"),
		PaksPath.Path,
		AesKey,
		TextureFormat.Replace(TEXT("."), TEXT("")),
		ExportAssetsPath.Path
	};
	FString ConsoleCommand = FString::Format(TEXT("{0} -path={1} -game=valorant -aes={2} *.uasset -export -noanim -nooverwrite -{3} -out={4}"), args);
	GEngine->Exec(nullptr, *ConsoleCommand);
}