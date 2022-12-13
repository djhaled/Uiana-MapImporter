#include "UianaImporter.h"
#include "UianaImporter.h"

#include "ActorEditorUtils.h"
#include "EditorClassUtils.h"
#include "EditorLevelUtils.h"
#include "HismActorCPP.h"
#include "IHeadMountedDisplay.h"
#include "Components/BrushComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "Engine/DecalActor.h"
#include "Engine/LevelStreamingAlwaysLoaded.h"
#include "Engine/SCS_Node.h"
#include "Engine/StaticMeshActor.h"
#include "Factories/BlueprintFactory.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/ComponentEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/StructureEditorUtils.h"
#include "Lightmass/LightmassCharacterIndirectDetailVolume.h"

#define LOCTEXT_NAMESPACE "UUianaImporter"

UUianaCPPDataSettings* UUianaImporter::Settings = nullptr;
FString UUianaImporter::Name = FString();
FString UUianaImporter::ValorantVersion = FString();
TArray<FString> UUianaImporter::UMaps = TArray<FString>();
FDirectoryPath UUianaImporter::PaksPath = FDirectoryPath();
FDirectoryPath UUianaImporter::FolderPath = FDirectoryPath();
FDirectoryPath UUianaImporter::ToolsPath = FDirectoryPath();
FDirectoryPath UUianaImporter::AssetsPath = FDirectoryPath();
FDirectoryPath UUianaImporter::ExportAssetsPath = FDirectoryPath();
FDirectoryPath UUianaImporter::ExportMapsPath = FDirectoryPath();
FDirectoryPath UUianaImporter::MaterialsPath = FDirectoryPath();
FDirectoryPath UUianaImporter::MaterialsOvrPath = FDirectoryPath();
FDirectoryPath UUianaImporter::ObjectsPath = FDirectoryPath();
FDirectoryPath UUianaImporter::ScenesPath = FDirectoryPath();
FDirectoryPath UUianaImporter::UMapsPath = FDirectoryPath();
FDirectoryPath UUianaImporter::UMapJsonPath = FDirectoryPath();
FDirectoryPath UUianaImporter::ActorsPath = FDirectoryPath();

UUianaImporter::UUianaImporter()
{
	UE_LOG(LogTemp, Error, TEXT("Uiana: Must initialize importer with parameters!"));
}

void UUianaImporter::Initialize(FString MapName, UUianaCPPDataSettings* InputSettings)
{
	Settings = InputSettings;
	FString RelativeContentDir = IPluginManager::Get().FindPlugin(TEXT("UianaCPP"))->GetContentDir();
	FString ContentDir = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*RelativeContentDir);
	PaksPath = Settings->PaksFolder;
	ValorantVersion = Settings->ValorantVersion;
	
	Name = MapName;
	// Create content directories
	UianaHelpers::CreateFolder(ToolsPath, ContentDir, "tools"); // tools_path
	UianaHelpers::CreateFolder(AssetsPath, ContentDir, "assets"); // importer_assets_path
	UianaHelpers::CreateFolder(ExportAssetsPath, Settings->ExportFolder.Path, "export"); // assets_path
	UianaHelpers::CreateFolder(ExportMapsPath, Settings->ExportFolder.Path, "maps"); // maps_path
	UianaHelpers::CreateFolder(FolderPath, ExportMapsPath.Path, "" + MapName);
	UianaHelpers::CreateFolder(MaterialsPath, FolderPath.Path, "materials");
	UianaHelpers::CreateFolder(MaterialsOvrPath, FolderPath.Path, "materials_ovr");
	UianaHelpers::CreateFolder(ObjectsPath, FolderPath.Path, "objects");
	UianaHelpers::CreateFolder(ScenesPath, FolderPath.Path, "scenes");
	UianaHelpers::CreateFolder(UMapsPath, FolderPath.Path, "umaps");
	UianaHelpers::CreateFolder(ActorsPath, FolderPath.Path, "actors");
	
	// Open umaps JSON file and read the UMaps to store
	FString UMapJSON;
	UMapJsonPath.Path = FPaths::Combine(AssetsPath.Path, "umaps.json");
	FFileHelper::LoadFileToString(UMapJSON, *UMapJsonPath.Path);
	TSharedPtr<FJsonObject> JsonParsed = UianaHelpers::ParseJson(UMapJSON);
	if (!JsonParsed.IsValid() || !JsonParsed->TryGetStringArrayField(MapName, UMaps))
	{
		UE_LOG(LogTemp, Error, TEXT("UIANA: Failed to deserialize umaps for %s"), *MapName);
	}
}

void UUianaImporter::ImportMap()
{
	UBPFL::ChangeProjectSettings();
	UBPFL::ExecuteConsoleCommand("r.DefaultFeature.LightUnits 0");
	UBPFL::ExecuteConsoleCommand("r.DynamicGlobalIlluminationMethod 0");
	TArray<FString> umapPaths;
	TArray<FString> levelPaths = {};
	TArray<FString> texturePaths = {};
	FFileManagerGeneric::Get().FindFiles(umapPaths, *(UMapsPath.Path), TEXT(".json"));
	UianaHelpers::AddPrefixPath(UMapsPath, umapPaths);
	if (NeedExport())
	{
		ExtractAssets(umapPaths);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: No need to extract PAK assets, skipping."));
	}
	if (!Settings->UseSubLevels)
	{
		levelPaths.Add(CreateNewLevel(Name));
	}
	// Clear level
	UEditorActorSubsystem* actorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	actorSubsystem->DestroyActors(actorSubsystem->GetAllLevelActors());
	if (Settings->ImportMaterials)
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Importing Textures!"));
		// Import textures first
		TArray<FString> matPaths, matOvrPaths;
		FFileManagerGeneric::Get().FindFiles(matPaths, *MaterialsPath.Path, TEXT(".json"));
		FFileManagerGeneric::Get().FindFiles(matOvrPaths, *MaterialsOvrPath.Path, TEXT(".json"));
		UianaHelpers::AddPrefixPath(MaterialsPath, matPaths);
		UianaHelpers::AddPrefixPath(MaterialsOvrPath, matOvrPaths);
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Found %d textures to import from path %s"), matPaths.Num(), *MaterialsPath.Path);
		GetTexturePaths(matPaths, texturePaths);
		GetTexturePaths(matOvrPaths, texturePaths);
		UBPFL::ImportTextures(texturePaths);
	
		// Import materials next
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Importing Materials!"));
		CreateMaterials(matPaths);
		CreateMaterials(matOvrPaths);
	}
	if (Settings->ImportMeshes)
	{
		TSet<FString> meshes;
		TArray<FString> meshRawPaths;
		// TODO: Try using LoadFileToStringArrayWithPredicate() or WithLineVisitor to reduce code complexity with blacklist
		FFileHelper::LoadFileToStringArray(meshRawPaths, *FPaths::Combine(FolderPath.Path, "_assets_objects.txt"));
		for (FString meshRawPath : meshRawPaths)
		{
			if (IsBlacklisted(FPaths::GetCleanFilename(meshRawPath))) continue;
			FString temp, temp1, meshPath;
			meshRawPath.Replace(TEXT("\\"), TEXT("/")).Split(TEXT("/"), &temp, &temp1);
			if (temp.Equals("Engine")) continue;
			temp1.Split(TEXT("/"), &temp, &meshPath);
			meshPath = FPaths::Combine(ExportAssetsPath.Path, "Game", meshPath);
			meshes.Emplace(FPaths::SetExtension(meshPath, TEXT(".pskx")));
			// UE_LOG(LogTemp, Display, TEXT("Uiana: Found mesh with result path %s from raw path %s filtered to %s"), *FPaths::SetExtension(meshPath, TEXT(".pskx")), *meshRawPath, *meshPath);
		}
		UBPFL::ImportMeshes(meshes, ObjectsPath.Path);
	}
	if (Settings->ImportBlueprints)
	{
		TArray<FString> bpPaths;
		FFileManagerGeneric::Get().FindFiles(bpPaths, *ActorsPath.Path, TEXT(".json"));
		UE_LOG(LogTemp, Display, TEXT("Uiana: Found %d BPs to import!"), bpPaths.Num());
		UianaHelpers::AddPrefixPath(ActorsPath, bpPaths);
		CreateBlueprints(bpPaths);
	}
	FScopedSlowTask UianaTask(umapPaths.Num(), LOCTEXT ("UianaTask", "Importing Map"));
	UianaTask.MakeDialog();
	// UWorld* world = GEditor->GetEditorWorldContext().World();
	UWorld* world = GEditor->PlayWorld;
	for (int i = umapPaths.Num() - 1; i >= 0; i--)
	{
		FString umapStr;
		FFileHelper::LoadFileToString(umapStr, *umapPaths[i]);
		TArray<TSharedPtr<FJsonValue>> umapData, umapFiltered;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(umapStr);
		if (!FJsonSerializer::Deserialize(JsonReader, umapData) || !umapData[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize umap %s"), *umapPaths[i]);
			continue;
		}
		FString umapName = FPaths::GetBaseFilename(umapPaths[i]);
		// TODO: Add level name to format text
		UianaTask.EnterProgressFrame(1, FText::Format(LOCTEXT("UianaTask", "Importing level {1}/{2}"), umapPaths.Num() - i, umapPaths.Num()));
		ImportUmap(umapData, umapName);
		if (Settings->UseSubLevels)
		{
			levelPaths.Add(CreateNewLevel(umapName));
		}
	}
	if (Settings->UseSubLevels)
	{
		UEditorLevelUtils::AddLevelsToWorld(world, levelPaths, ULevelStreamingAlwaysLoaded::StaticClass());
	}
	if (Settings->ImportMeshes)
	{
		TArray<FString> objPaths;
		FFileManagerGeneric::Get().FindFiles(objPaths, *ObjectsPath.Path, TEXT(".json"));
		UianaHelpers::AddPrefixPath(ObjectsPath, objPaths);
		for (const FString objPath : objPaths)
		{
			TArray<TSharedPtr<FJsonValue>> objData;
			FString objStr;
			FFileHelper::LoadFileToString(objStr, *objPath);
			const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(objStr);
			if (!FJsonSerializer::Deserialize(JsonReader, objData) || !objData[0].IsValid())
			{
				UE_LOG(LogScript, Warning, TEXT("Uiana: Failed to deserialize obj %s"), *objPath);
				continue;
			}
			for (TSharedPtr<FJsonValue> component : objData)
			{
				if (component->IsNull() || !component.IsValid()) continue;
				const TSharedPtr<FJsonObject> componentObj = component->AsObject();
				if (!componentObj->HasField("Properties")) continue;
				const TSharedPtr<FJsonObject> componentProps = componentObj->GetObjectField("Properties");
				if (componentObj->HasTypedField<EJson::String>("Type"))
				{
					UStaticMesh* mesh = Cast<UStaticMesh>(UEditorAssetLibrary::LoadAsset(FPaths::Combine("/Game/ValorantContent/Meshes/", componentObj->GetStringField("Name"))));
					if (mesh == nullptr)
					{
						UE_LOG(LogScript, Warning, TEXT("Uiana: Failed to import mesh to modify: %s"), *componentObj->GetStringField("Name"));
						continue;
					}
					if (componentObj->GetStringField("Type").Equals("StaticMesh"))
					{
						double lightmapRes = round(256 * Settings->LightmapResolutionMultiplier / 4) * 4;
						int lightmapCoord = 1;
						if (componentProps.IsValid() && componentProps->HasTypedField<EJson::Number>("LightMapCoordinateIndex"))
						{
							lightmapCoord = componentProps->GetIntegerField("LightMapCoordinateIndex");
						}
						if (componentProps.IsValid() && componentProps->HasTypedField<EJson::Number>("LightMapResolution"))
						{
							lightmapRes = round(componentProps->GetNumberField("LightMapResolution") * Settings->LightmapResolutionMultiplier / 4) * 4;
						}
						UE_LOG(LogTemp, Display, TEXT("Uiana: Mesh %s has lightmap resolution %d and lightmapCoord %d"), *componentObj->GetStringField("Name"), static_cast<int32>(lightmapRes), lightmapCoord);
						mesh->SetLightMapResolution(lightmapRes);
						mesh->SetLightMapCoordinateIndex(lightmapCoord);
					}
					if (componentObj->GetStringField("Type").Equals("BodySetup") && componentProps.IsValid() && componentProps->HasField("CollisionTraceFlag"))
					{
						// TODO: Verify this works vs setting the editor property!
						UBodySetup* bodySetup = mesh->GetBodySetup();
						bodySetup->CollisionTraceFlag = UianaHelpers::ParseCollisionTrace(componentProps->GetStringField("CollisionTraceFlag"));
						mesh->SetBodySetup(bodySetup);
					}
				}
			}
		}
	}
}

void UUianaImporter::ExtractAssets(TArray<FString> umapPaths)
{
	UE_LOG(LogTemp, Warning, TEXT("Uiana: Extracting assets!"));
	CUE4Extract(UMapsPath);
	UModelExtract();
	UE_LOG(LogTemp, Warning, TEXT("Uiana: Extracted %d umaps"), umapPaths.Num());
	TArray<FString> actorPaths, objPaths, matOvrPaths;
	for (FString umapPath : umapPaths)
	{
		FString umapStr;
		// FString umapPath = FPaths::Combine(UMapsPath.Path, umapName);
		FFileHelper::LoadFileToString(umapStr, *umapPath);
		TArray<TSharedPtr<FJsonValue>> umapRaw, umapFiltered;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(umapStr);
		if (!FJsonSerializer::Deserialize(JsonReader, umapRaw) || !umapRaw[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize umap %s"), *umapPath);
			continue;
		}
		
		// Filter umap
		const TArray<FString> decalTypes = {"decalcomponent"};
		const TArray<FString> meshTypes = {"staticmesh", "staticmeshcomponent", "instancedstaticmeshcomponent",
			"hierarchicalinstancedstaticmeshcomponent"};
		const TArray<FString> genTypes = {"pointlightcomponent", "postprocessvolume", "culldistancevolume",
			"scenecomponent", "lightmasscharacterindirectdetailvolume", "brushcomponent", "precomputedvisibilityvolume",
			"rectlightcomponent", "spotlightcomponent", "skylightcomponent", "scenecapturecomponentcube",
			"lightmassimportancevolume", "billboardcomponent", "directionallightcomponent",
			"exponentialheightfogcomponent", "lightmassportalcomponent", "spherereflectioncapturecomponent"};
		for (TSharedPtr<FJsonValue> component : umapRaw)
		{
			const TSharedPtr<FJsonObject> obj = component.Get()->AsObject();
			const FString typeLower = obj->GetStringField("Type").ToLower();
			if (meshTypes.Contains(typeLower) && obj->HasField("Properties"))
			{
				umapFiltered.Add(component);				
			}
			else if (genTypes.Contains(typeLower) || decalTypes.Contains(typeLower))
			{
				umapFiltered.Add(component);
			}
			else if (typeLower.EndsWith("_c"))
			{
				umapFiltered.Add(component);
			}
		}
		
		// Save cleaned-up JSON
		UianaHelpers::SaveJson(umapFiltered, umapPath);

		GetObjects(actorPaths, objPaths, matOvrPaths, umapFiltered);
	}
	// Process blueprint actors
	UE_LOG(LogTemp, Display, TEXT("Uiana: Importing BP Actors!"));
	const FString actorPathsFilepath = FPaths::Combine(FolderPath.Path, "_assets_actors.txt");
	FFileHelper::SaveStringArrayToFile(actorPaths, *actorPathsFilepath);
	CUE4Extract(ActorsPath, actorPathsFilepath);
	actorPaths.Empty();
	FFileManagerGeneric::Get().FindFiles(actorPaths, *(ActorsPath.Path), TEXT(".json"));
	for (FString actorPath : actorPaths)
	{
		FString actorStr;
		FFileHelper::LoadFileToString(actorStr, *(ActorsPath.Path + "/" + actorPath));
		TArray<TSharedPtr<FJsonValue>> actorObjs;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(actorStr);
		if (!FJsonSerializer::Deserialize(JsonReader, actorObjs) || !actorObjs[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize actor %s"), *actorPath);
			continue;
		}
		TArray<FString> temp;
		GetObjects(temp, objPaths, matOvrPaths, actorObjs);
	}
	// Save asset lists
	const FString objPathsFilepath = FPaths::Combine(FolderPath.Path, "_assets_objects.txt");
	const FString matPathsFilepath = FPaths::Combine(FolderPath.Path, "_assets_materials_ovr.txt");
	UE_LOG(LogTemp, Display, TEXT("Uiana: Saving asset file with %d assets on path: %s"), objPaths.Num(), *objPathsFilepath);
	FFileHelper::SaveStringArrayToFile(objPaths, *objPathsFilepath);
	FFileHelper::SaveStringArrayToFile(matOvrPaths, *matPathsFilepath);
	CUE4Extract(ObjectsPath, objPathsFilepath);
	CUE4Extract(MaterialsOvrPath, matPathsFilepath);
	
	// Get models now
	UE_LOG(LogTemp, Display, TEXT("Uiana: Getting models!"));
	TArray<FString> modelNames;
	TArray<FString> matPaths = {};
	FFileManagerGeneric::Get().FindFiles(modelNames, *(ObjectsPath.Path), TEXT(".json"));
	for (FString modelName : modelNames)
	{
		const FString modelPath = FPaths::Combine(ObjectsPath.Path, modelName);
		FString jsonStr;
		FFileHelper::LoadFileToString(jsonStr, *modelPath);
		TArray<TSharedPtr<FJsonValue>> modelObjs;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(jsonStr);
		if (!FJsonSerializer::Deserialize(JsonReader, modelObjs) || !modelObjs[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize model %s"), *modelPath);
			continue;
		}
		
		// Save cleaned-up JSON
		UianaHelpers::SaveJson(modelObjs, modelPath);
		
		// Get Object Materials
		for (TSharedPtr<FJsonValue> modelObject : modelObjs)
		{
			if (modelObject.Get()->AsObject()->GetStringField("Type").Equals("StaticMesh"))
			{
				const TArray<TSharedPtr<FJsonValue>> modelMats = modelObject.Get()->AsObject()->GetObjectField("Properties")->GetArrayField("StaticMaterials");
				for(const TSharedPtr<FJsonValue> mat : modelMats)
				{
					const TSharedPtr<FJsonObject> obj = mat.Get()->AsObject();
					if (obj->HasTypedField<EJson::Object>("MaterialInterface"))
					{
						const FString path = obj->GetObjectField("MaterialInterface")->GetStringField("ObjectPath");
						matPaths.AddUnique(FPaths::Combine(FPaths::GetPath(path), FPaths::GetBaseFilename(path)).Replace(TEXT("/"), TEXT("\\")));
					}
				}
			}
		}
	}
	// Save material list + all assets list
	TArray<FString> allPaths = {};
	UianaHelpers::AddAllAssetPath(allPaths, objPaths);
	UianaHelpers::AddAllAssetPath(allPaths, matPaths);
	UianaHelpers::AddAllAssetPath(allPaths, matOvrPaths);
	
	const FString matListFilepath = FPaths::Combine(FolderPath.Path, "_assets_materials.txt");
	const FString allListFilepath = FPaths::Combine(FolderPath.Path, "all_assets.txt");
	FFileHelper::SaveStringArrayToFile(matPaths, *matListFilepath);
	FFileHelper::SaveStringArrayToFile(allPaths, *allListFilepath);
	CUE4Extract(MaterialsPath, matListFilepath);
	// Write exported.yo to indicate have exported
	FUianaExport exportInfo;
	exportInfo.version = ValorantVersion;
	FString exportStr;
	FJsonObjectConverter::UStructToJsonObjectString<FUianaExport>(exportInfo, exportStr);
	FFileHelper::SaveStringToFile(exportStr, *FPaths::Combine(FolderPath.Path, "exported.yo"));
	FFileHelper::SaveStringToFile(exportStr, *FPaths::Combine(ExportAssetsPath.Path, "exported.yo"));
}

void UUianaImporter::CUE4Extract(FDirectoryPath ExportDir, FString AssetList)
{
	TArray<FStringFormatArg> args = {
		PaksPath.Path,
		AesKey,
		ExportDir.Path,
		Name,
		AssetList,
		UMapJsonPath.Path
	};
	FString ConsoleCommand = FString::Format(TEXT("--game-directory \"{0}\" --aes-key {1} --export-directory \"{2}\" --map-name {3} --file-list {4} --game-umaps \"{5}\""), args);
	UE_LOG(LogTemp, Warning, TEXT("%s %s"), *FPaths::Combine(ToolsPath.Path, "cue4extractor.exe"), *ConsoleCommand);
	FProcHandle handle = FPlatformProcess::CreateProc(*FPaths::Combine(ToolsPath.Path, "cue4extractor.exe"), *ConsoleCommand, false, false, false, nullptr, 1, nullptr, nullptr);
	if (handle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Ran CU4 successfully!"));
		FPlatformProcess::WaitForProc(handle);
	}
}

void UUianaImporter::CUE4Extract(FDirectoryPath ExportDir)
{
	TArray<FStringFormatArg> args = {
		PaksPath.Path,
		AesKey,
		ExportDir.Path,
		Name,
		UMapJsonPath.Path
	};
	FString ConsoleCommand = FString::Format(TEXT("--game-directory \"{0}\" --aes-key {1} --export-directory \"{2}\" --map-name {3} --game-umaps \"{4}\""), args);
	UE_LOG(LogTemp, Warning, TEXT("%s %s"), *FPaths::Combine(ToolsPath.Path, "cue4extractor.exe"), *ConsoleCommand);
	FProcHandle handle = FPlatformProcess::CreateProc(*FPaths::Combine(ToolsPath.Path, "cue4extractor.exe"), *ConsoleCommand, false, false, false, nullptr, 1, nullptr, nullptr);
	if (handle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Ran CU4 successfully!"));
		FPlatformProcess::WaitForProc(handle);
	}
}

void UUianaImporter::UModelExtract()
{
	const FString allAssetPath = FPaths::Combine(FolderPath.Path, "all_assets.txt");
	TArray<FStringFormatArg> args = {
		PaksPath.Path,
		AesKey,
		allAssetPath,
		TextureFormat.Replace(TEXT("."), TEXT("")),
		ExportAssetsPath.Path
	};
	const FString ConsoleCommand = FString::Format(TEXT("-path=\"{0}\" -game=valorant -aes={1} -files=\"{2}\" -export -{3} -out=\"{4}\""), args);
	UE_LOG(LogTemp, Warning, TEXT("%s %s"), *FPaths::Combine(ToolsPath.Path, "umodel.exe"), *ConsoleCommand);
	FProcHandle handle = FPlatformProcess::CreateProc(*FPaths::Combine(ToolsPath.Path, "umodel.exe"), *ConsoleCommand, false, false, false, nullptr, 1, nullptr, nullptr);
	FPlatformProcess::WaitForProc(handle);
}

FString UUianaImporter::CreateNewLevel(const FString levelName)
{
	// Get initial name
	FString initialName, temp;
	levelName.Split(TEXT("_"), &initialName, &temp);
	if (initialName.Equals("")) initialName = levelName;
	TArray<FStringFormatArg> args = {initialName, levelName};
	const FString levelPath = FString::Format(TEXT("/Game/ValorantContent/Maps/{0}/{1}"), args);
	UE_LOG(LogTemp, Warning, TEXT("Uiana: Creating new level at path: %s"), *levelPath);
	UEditorAssetLibrary::LoadAsset(levelPath);
	ULevelEditorSubsystem* editorSubsystem = GEditor->GetEditorSubsystem<ULevelEditorSubsystem>();
	editorSubsystem->NewLevel(levelPath);
	return levelPath;
}

void UUianaImporter::GetTexturePaths(const TArray<FString> matPaths, TArray<FString> &texturePaths)
{
	for (const FString matPath : matPaths)
	{
		FString jsonStr;
		FFileHelper::LoadFileToString(jsonStr, *matPath);
		TArray<TSharedPtr<FJsonValue>> matObjs;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(jsonStr);
		if (!FJsonSerializer::Deserialize(JsonReader, matObjs) || !matObjs[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize material %s"), *matPath);
			continue;
		}
		for (TSharedPtr<FJsonValue> mat : matObjs)
		{
			const TSharedPtr<FJsonObject> obj = mat.Get()->AsObject();
			if (obj->HasField("Properties") && obj->GetObjectField("Properties")->HasField("TextureParameterValues"))
			{
				const TArray<TSharedPtr<FJsonValue>> texParams = obj->GetObjectField("Properties")->GetArrayField("TextureParameterValues");
				for (auto param : texParams)
				{
					const TSharedPtr<FJsonObject> paramObj = param->AsObject();
					if (!paramObj->HasTypedField<EJson::Object>("ParameterValue")) continue;
					const FString objectPath = paramObj->GetObjectField("ParameterValue")->GetStringField("ObjectPath");
					FString objectPathName = FPaths::GetBaseFilename(objectPath, false) + TextureFormat;
					const FString texturePath = FPaths::Combine(ExportAssetsPath.Path,
						objectPathName.Replace(
						TEXT("ShooterGame"), TEXT("Game")).Replace(
							TEXT("/Content"), TEXT("")));
					texturePaths.AddUnique(texturePath);
				}
			}
		}
	}
}

void UUianaImporter::CreateMaterials(const TArray<FString> matPaths)
{
	for(FString matPath : matPaths)
	{
		// TODO: Simplify CreateMaterial() and GetTexturePaths() since they share same stuff
		// TODO: Make this a function already dangnabbit
		FString jsonStr;
		FFileHelper::LoadFileToString(jsonStr, *matPath);
		TArray<TSharedPtr<FJsonValue>> matObjs;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(jsonStr);
		if (!FJsonSerializer::Deserialize(JsonReader, matObjs) || !matObjs[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize material %s"), *matPath);
			continue;
		}
		const TSharedPtr<FJsonObject> mat = matObjs.Pop()->AsObject();
		if (!mat->HasField("Properties") || !mat->HasField("Name")) // Not sure about Name skip
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Skipping due to missing properties for material %s"), *matPath);
			continue;
		}

		TArray<FStringFormatArg> args = {mat->GetStringField("Name"), mat->GetStringField("Name")};
		const FString localMatPath = FString::Format(TEXT("/Game/ValorantContent/Materials/{0}.{1}"), args);
		UMaterialInstanceConstant* matInstance = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset(localMatPath));
		if (matInstance == nullptr)
		{
			UMaterialInstanceConstantFactoryNew* factory = NewObject<UMaterialInstanceConstantFactoryNew>();
			IAssetTools& assetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
			matInstance = static_cast<UMaterialInstanceConstant*>(assetTools.CreateAsset(mat->GetStringField("Name"), "/Game/ValorantContent/Materials/", UMaterialInstanceConstant::StaticClass(), factory));
			UE_LOG(LogTemp, Warning, TEXT("Uiana: Created missing material %s"), *localMatPath);
		}
		
		FString matParent = "BaseEnv_MAT_V4";
		if (mat->GetObjectField("Properties")->HasField("Parent") && !mat->GetObjectField("Properties")->GetObjectField("Parent")->GetStringField("ObjectName").IsEmpty())
		{
			FString temp;
			mat->GetObjectField("Properties")->GetObjectField("Parent")->GetStringField("ObjectName").Split(TEXT(" "), &temp, &matParent);
		}
		UMaterialInstanceConstant* parentInstance = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/UianaCPP/Materials/" + matParent));
		if (parentInstance == nullptr)
		{
			parentInstance = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/UianaCPP/Materials/BaseEnv_MAT_V4"));
		}
		matInstance->SetParentEditorOnly(parentInstance);
		SetMaterial(mat, matInstance);
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Created material %s at %s"), *matPath, *localMatPath);
	}
}

void UUianaImporter::SetMaterial(const TSharedPtr<FJsonObject> matData, UMaterialInstanceConstant* mat)
{
	SetTextures(matData, mat);
	SetMaterialSettings(matData->GetObjectField("Properties"), mat);
	const TSharedPtr<FJsonObject> matProps = matData.Get()->GetObjectField("Properties");
	if (matProps.Get()->HasTypedField<EJson::Object>("BasePropertyOverrides"))
	{
		FMaterialInstanceBasePropertyOverrides overrides = SetBasePropertyOverrides(matProps.Get()->GetObjectField("BasePropertyOverrides"));
		mat->BasePropertyOverrides = overrides;
	}
	if (matProps.Get()->HasField("StaticParameters"))
	{
		if (matProps.Get()->GetObjectField("StaticParameters")->HasField("StaticSwitchParameters"))
		{
			for (TSharedPtr<FJsonValue> param : matProps.Get()->GetObjectField("StaticParameters")->GetArrayField("StaticSwitchParameters"))
			{
				const TSharedPtr<FJsonObject> paramObj = param.Get()->AsObject();
				const TSharedPtr<FJsonObject> paramInfo = paramObj.Get()->GetObjectField("ParameterInfo");
				UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(mat, FName(*paramInfo.Get()->GetStringField("Name").ToLower()), paramObj.Get()->GetBoolField("Value"));
			}
		}
		if (matProps.Get()->GetObjectField("StaticParameters")->HasField("StaticComponentMaskParameters"))
		{
			for (TSharedPtr<FJsonValue> param : matProps.Get()->GetObjectField("StaticParameters")->GetArrayField("StaticComponentMaskParameters"))
			{
				const TArray<FString> maskList = {"R", "G", "B"};
				for (FString mask : maskList)
				{
					UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(mat, FName(*mask), param.Get()->AsObject().Get()->GetBoolField(mask));
				}
			}
		}
	}
	if (matProps.Get()->HasField("ScalarParameterValues"))
	{
		for (TSharedPtr<FJsonValue> param : matProps.Get()->GetArrayField("ScalarParameterValues"))
		{
			const TSharedPtr<FJsonObject> paramObj = param.Get()->AsObject();
			const TSharedPtr<FJsonObject> paramInfo = paramObj.Get()->GetObjectField("ParameterInfo");
			UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(mat, FName(*paramInfo.Get()->GetStringField("Name").ToLower()), param.Get()->AsObject()->GetNumberField("ParameterValue"));
		}
	}
	if (matProps.Get()->HasField("VectorParameterValues"))
	{
		for (TSharedPtr<FJsonValue> param : matProps.Get()->GetArrayField("VectorParameterValues"))
		{
			FLinearColor vec;
			const TSharedPtr<FJsonObject> paramVal = param.Get()->AsObject()->GetObjectField("ParameterValue");
			vec.R = paramVal.Get()->GetNumberField("R");
			vec.G = paramVal.Get()->GetNumberField("G");
			vec.B = paramVal.Get()->GetNumberField("B");
			UMaterialEditingLibrary::SetMaterialInstanceVectorParameterValue(mat, FName(*param.Get()->AsObject()->GetObjectField("ParameterInfo")->GetStringField("Name").ToLower()), vec);
		}
	}
	UMaterialEditingLibrary::UpdateMaterialInstance(mat);
	// TODO: Homogenize the Scalar/Vector/TextureParameterValues sections
}

void UUianaImporter::SetTextures(const TSharedPtr<FJsonObject> matData, UMaterialInstanceConstant* mat)
{
	if (!matData->GetObjectField("Properties")->HasField("TextureParameterValues"))
	{
		return;
	}
	const TArray<TSharedPtr<FJsonValue>> matTexParams = matData->GetObjectField("Properties")->GetArrayField("TextureParameterValues");
	for (const TSharedPtr<FJsonValue> param : matTexParams)
	{
		// TODO: Cleanup 1-line continues to be less big
		if (!param.Get()->AsObject()->HasField("ParameterValue"))
		{
			continue;
		}
		FString textureGamePath = FPaths::ChangeExtension(param.Get()->AsObject()->GetObjectField("ParameterValue")->GetStringField("ObjectPath"), TextureFormat);
		FString localPath = FPaths::Combine(ExportAssetsPath.Path, textureGamePath.Replace(
						TEXT("ShooterGame"), TEXT("Game")).Replace(
							TEXT("/Content"), TEXT("")));
		const TSharedPtr<FJsonObject> paramInfo = param.Get()->AsObject()->GetObjectField("ParameterInfo");
		FString paramName = param.Get()->AsObject()->GetObjectField("ParameterInfo")->GetStringField("Name").ToLower();
		if (FPaths::DirectoryExists(FPaths::GetPath(localPath)))
		{
			const FString texturePath = FPaths::Combine("/Game/ValorantContent/Textures/", FPaths::GetBaseFilename(localPath));
			UTexture* loadedTexture = static_cast<UTexture*>(
					UEditorAssetLibrary::LoadAsset(texturePath));
			if (loadedTexture == nullptr) continue;
			UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(mat, *paramName, loadedTexture);
		}
		else UE_LOG(LogTemp, Warning, TEXT("Uiana: Missing texture at expected directory %s"), *FPaths::GetPath(localPath));
	}
}

FMaterialInstanceBasePropertyOverrides UUianaImporter::SetBasePropertyOverrides(const TSharedPtr<FJsonObject> matProps)
{
	FMaterialInstanceBasePropertyOverrides overrides = FMaterialInstanceBasePropertyOverrides();
	// Loop through all JSON values (type <FString, TSharedPtr<FJsonValue>>)
	for (auto const& prop : matProps->Values)
	{
		TSharedPtr<FJsonValue> propValue = prop.Value;
		const FName propName = FName(*prop.Key);
		FProperty* objectProp = overrides.StaticStruct()->FindPropertyByName(propName);
		if (objectProp == nullptr) continue;
		const EJson propType = propValue.Get()->Type;
		if (propType == EJson::Number)
		{
			double value = prop.Value.Get()->AsNumber();
			if (prop.Key.Equals("InfluenceRadius") && propValue.Get()->AsNumber() == 0)
			{
				value = 14680;
			}
			// TODO: Fix importing float properties, cannot set OpacityClipValue due to some assertion
			if (!UianaHelpers::SetStructProperty<FDoubleProperty, double>(&overrides, objectProp, value))// && !UianaHelpers::SetStructProperty<FFloatProperty, float>(&overrides, objectProp, value))
				UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to set property %s of type number for Override struct!"), *prop.Key);
			continue;
		}
		if (propType == EJson::Boolean)
		{
			bool value = prop.Value.Get()->AsBool();
			if (!UianaHelpers::SetStructProperty<FBoolProperty, bool>(&overrides, objectProp, value))
				UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to set property %s of type bool for Override struct!"), *prop.Key);
			continue;
		}
		if (propType == EJson::String && propValue.Get()->AsString().Contains("::"))
		{
			FString temp, splitResult;
			propValue.Get()->AsString().Split("::", &temp, &splitResult);
			FJsonValueString valString = FJsonValueString(splitResult);
			propValue = MakeShareable<FJsonValueString>(&valString);
		}
		if (objectProp->GetClass()->GetName().Equals("FLinearColor"))
		{
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			const TArray<FName> params = {"R", "G", "B"};
			if (!UianaHelpers::SetStructPropertiesFromJson<FDoubleProperty, double>(&overrides, objectProp, obj, params))
				UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to set property %s of type FLinearColor for Override struct!"), *prop.Key);
		}
		else if (objectProp->GetClass()->GetName().Equals("FVector4"))
		{
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			const TArray<FName> params = {"X", "Y", "Z", "W"};
			if(!UianaHelpers::SetStructPropertiesFromJson<FDoubleProperty, double>(&overrides, objectProp, obj, params))
				UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to set property %s of type FVector4 for Override struct!"), *prop.Key);
		}
		else if (objectProp->GetClass()->GetName().Contains("Color"))
		{
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			const TArray<FName> params = {"R", "G", "B", "A"};
			if (!UianaHelpers::SetStructPropertiesFromJson<FDoubleProperty, double>(&overrides, objectProp, obj, params))
				UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to set property %s of type Color for Override struct!"), *prop.Key);
		}
		else
		{
			if (propType == EJson::String)
			{
				const FString dataStr = propValue.Get()->AsString();
				if (prop.Key.Equals("ShadingModel"))
				{
					overrides.ShadingModel = UianaHelpers::ParseShadingModel(dataStr);
				}
				else if (prop.Key.Equals("BlendMode"))
				{
					overrides.BlendMode = UianaHelpers::ParseBlendMode(dataStr);
				}
				else
				{
					UE_LOG(LogTemp, Display, TEXT("Uiana: Needed Override is String with value %s and CPP Class %s %s"), *propValue.Get()->AsString(), *objectProp->GetNameCPP(), *objectProp->GetCPPTypeForwardDeclaration());
				}
			}
			else UE_LOG(LogTemp, Display, TEXT("Uiana: Need to set Override %s somehow!"), *prop.Key);
		}
	}
	return overrides;
}

void UUianaImporter::SetMaterialSettings(const TSharedPtr<FJsonObject> matProps, UMaterialInstanceConstant* mat)
{
	// Loop through all JSON values (type <FString, TSharedPtr<FJsonValue>>)
	for (auto const& prop : matProps->Values)
	{
		TSharedPtr<FJsonValue> propValue = prop.Value;
		const FName propName = FName(*prop.Key);
		const FProperty* objectProp = PropertyAccessUtil::FindPropertyByName(propName, mat->GetClass());
		if (objectProp == nullptr) continue;
		const EJson propType = propValue.Get()->Type;
		if (propType == EJson::Number || propType == EJson::Boolean)
		{
			if (prop.Key.Equals("InfluenceRadius") && propValue.Get()->AsNumber() == 0)
			{
				FObjectEditorUtils::SetPropertyValue(mat, propName, 14680);
				continue;
			}
			FObjectEditorUtils::SetPropertyValue(mat, propName, prop.Value.Get()->AsNumber());
			continue;
		}
		if (propType == EJson::String && propValue.Get()->AsString().Contains("::"))
		{
			FString temp, splitResult;
			propValue.Get()->AsString().Split("::", &temp, &splitResult);
			FJsonValueString valString = FJsonValueString(splitResult);
			propValue = MakeShareable<FJsonValueString>(&valString);
		}
		if (objectProp->GetClass()->GetName().Equals("FLinearColor"))
		{
			FLinearColor color;
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			color.R = obj->GetNumberField("R");
			color.G = obj->GetNumberField("G");
			color.B = obj->GetNumberField("B");
			FObjectEditorUtils::SetPropertyValue(mat, propName, color);
		}
		else if (objectProp->GetClass()->GetName().Equals("FVector4"))
		{
			FVector4 vector;
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			vector.X = obj->GetNumberField("X");
			vector.Y = obj->GetNumberField("Y");
			vector.Z = obj->GetNumberField("Z");
			vector.W = obj->GetNumberField("W");
			FObjectEditorUtils::SetPropertyValue(mat, propName, vector);
		}
		else if (objectProp->GetClass()->GetName().Contains("Color"))
		{
			FColor color;
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			color.R = obj->GetNumberField("R");
			color.G = obj->GetNumberField("G");
			color.B = obj->GetNumberField("B");
			color.A = obj->GetNumberField("A");
			FObjectEditorUtils::SetPropertyValue(mat, propName, color);
		}
		else if (propType == EJson::Object)
		{
			if (prop.Key.Equals("IESTexture"))
			{
				FString temp, newTextureName;
				propValue.Get()->AsObject()->GetStringField("ObjectName").Split("_", &temp, &newTextureName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
				FString assetPath = "/UianaCPP/IESProfiles/" + newTextureName + "." + newTextureName;
				UTexture* newTexture = static_cast<UTexture*>(UEditorAssetLibrary::LoadAsset(assetPath));
				FObjectEditorUtils::SetPropertyValue(mat, propName, newTexture);
			}
			else if (prop.Key.Equals("Cubemap"))
			{
				FString newCubemapName = propValue.Get()->AsObject()->GetStringField("ObjectName").Replace(TEXT("TextureCube "), TEXT(""));
				FString assetPath = "/UianaCPP/CubeMaps/" + newCubemapName + "." + newCubemapName;
				// TODO: Convert all static_cast with UObjects to Cast<>()
				UTextureCube* newCube = Cast<UTextureCube, UObject>(UEditorAssetLibrary::LoadAsset(assetPath));
				FObjectEditorUtils::SetPropertyValue(mat, propName, newCube);
			}
			else if (prop.Key.Equals("DecalMaterial"))
			{
				// TODO: Implement remaining object imports. Line 303 in main.py
				FString decalPath = FPaths::GetPath(propValue.Get()->AsObject()->GetStringField("ObjectPath"));
				UMaterialInstanceConstant* decalMat = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Materials/" + decalPath + "." + decalPath));
				UDecalComponent decalComponent;
				decalComponent.SetMaterial(0, mat);
				decalComponent.SetDecalMaterial(decalMat);
			}
			else if (prop.Key.Equals("DecalSize"))
			{
				FVector vec;
				const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
				vec.X = obj->GetNumberField("X");
				vec.Y = obj->GetNumberField("Y");
				vec.Z = obj->GetNumberField("Z");
				FObjectEditorUtils::SetPropertyValue(mat, propName, vec);
			}
			else if (prop.Key.Equals("StaticMesh"))
			{
				FString meshName;
				if (propValue->AsObject()->TryGetStringField("ObjectName", meshName))
				{
					FString name = meshName.Replace(TEXT("StaticMesh "), TEXT(""), ESearchCase::CaseSensitive);
					UStaticMesh* mesh = static_cast<UStaticMesh*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Meshes/" + name));
					FObjectEditorUtils::SetPropertyValue(mat, propName, mesh);
				}
			}
			else if (prop.Key.Equals("BoxExtent"))
			{
				FVector vec;
				const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
				vec.X = obj->GetNumberField("X");
				vec.Y = obj->GetNumberField("Y");
				vec.Z = obj->GetNumberField("Z");
				FObjectEditorUtils::SetPropertyValue(mat, "BoxExtent", vec);
			}
			else if (prop.Key.Equals("LightmassSettings"))
			{
				// TODO: Investigate why lightmass settings do not seem to be getting applied although no error!
				FLightmassMaterialInterfaceSettings lightmassSettings;
				FJsonObjectConverter::JsonObjectToUStruct(propValue.Get()->AsObject().ToSharedRef(), &lightmassSettings);
				if (!FObjectEditorUtils::SetPropertyValue(mat, "LightmassSettings", lightmassSettings)) UE_LOG(LogTemp, Error, TEXT("Uiana: Failed to set lightmass settings!"));
			}
		}
		else if (propType == EJson::Array && prop.Key.Equals("OverrideMaterials"))
		{
			TArray<UMaterialInstanceConstant*> overrideMats;
			UMaterialInstanceConstant* loaded = nullptr;
			for (TSharedPtr<FJsonValue, ESPMode::ThreadSafe> ovrMat : propValue.Get()->AsArray())
			{
				if (!ovrMat.IsValid() || ovrMat.Get()->IsNull())
				{
					overrideMats.Add(nullptr);
					continue;
				}
				const TSharedPtr<FJsonObject> matData = ovrMat.Get()->AsObject();
				FString temp, objName;
				matData.Get()->GetStringField("ObjectName").Split(" ", &temp, &objName, ESearchCase::Type::IgnoreCase, ESearchDir::FromEnd);
				if (matData->HasTypedField<EJson::Object>("ObjectName")) UE_LOG(LogTemp, Error, TEXT("Uiana: Material Override Material ObjectName is not a string!"));
				if (objName.Contains("MaterialInstanceDynamic")) continue;
				loaded = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/UianaCPP/Materials/" + objName));
				if (loaded == nullptr) loaded = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Materials/" + objName));
				overrideMats.Add(loaded);
			}
			FObjectEditorUtils::SetPropertyValue(mat, propName, overrideMats);
		}
		else
		{
			if (propType == EJson::Array)
			{
				const TArray<TSharedPtr<FJsonValue>> parameterArray = propValue.Get()->AsArray();
				if (prop.Key.Equals("TextureStreamingData"))
				{
					TArray<FMaterialTextureInfo> textures;
					for (const TSharedPtr<FJsonValue> texture : parameterArray)
					{
						FMaterialTextureInfo textureInfo;
						const TSharedPtr<FJsonObject> textureObj = texture.Get()->AsObject();
						FJsonObjectConverter::JsonObjectToUStruct(textureObj.ToSharedRef(), &textureInfo);
						textures.Add(textureInfo);
					}
					mat->SetTextureStreamingData(textures);
				}
				else
				{
					if (prop.Key.Equals("ScalarParameterValues") || prop.Key.Equals("VectorParameterValues") || prop.Key.Equals("TextureParameterValues")) continue;
					for (const TSharedPtr<FJsonValue> parameter : parameterArray)
					{
						const TSharedPtr<FJsonObject> paramObj = parameter.Get()->AsObject();
						// const TSharedPtr<FJsonObject> paramInfo = paramObj.Get()->GetObjectField("ParameterInfo");
						// FMaterialParameterInfo info;
						// FJsonObjectConverter::JsonObjectToUStruct(paramInfo.ToSharedRef(), &info);
						// if (prop.Key.Equals("ScalarParameterValues"))
						// {
						// 	double paramValue = paramObj.Get()->GetNumberField("ParameterValue");
						// 	mat->SetScalarParameterValueEditorOnly(info, paramValue);
						// }
						// else if (prop.Key.Equals("VectorParameterValues"))
						// {
						// 	FLinearColor paramValue;
						// 	const TSharedPtr<FJsonObject> vectorParams = paramObj.Get()->GetObjectField("ParameterValue");
						// 	paramValue.R = vectorParams.Get()->GetNumberField("R");
						// 	paramValue.G = vectorParams.Get()->GetNumberField("G");
						// 	paramValue.B = vectorParams.Get()->GetNumberField("B");
						// 	paramValue.A = vectorParams.Get()->GetNumberField("A");
						// 	mat->SetVectorParameterValueEditorOnly(info, paramValue);
						// }
						// else if (prop.Key.Equals("TextureParameterValues"))
						// {
						// 	FString temp, textureName;
						// 	paramObj.Get()->GetObjectField("ParameterValue").Get()->GetStringField("ObjectName").Split(TEXT(" "), &temp, &textureName);
						// 	UTexture* paramValue = static_cast<UTexture*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Textures/" + textureName));
						// 	mat->SetTextureParameterValueEditorOnly(info, paramValue);
						// }
						// else
						// {
						FString OutputString;
						TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
						FJsonSerializer::Serialize(paramObj.ToSharedRef(), Writer);
						UE_LOG(LogTemp, Warning, TEXT("Uiana: Array Material Property unaccounted for with value: %s"), *OutputString);	
						// }
					}	
				}
			}
			else UE_LOG(LogTemp, Warning, TEXT("Uiana: Need to set Material Property %s of unknown type!"), *prop.Key);
		}
	}
}

bool UUianaImporter::NeedExport()
{
	FString exportCheckPath = FPaths::Combine(FolderPath.Path, "exported.yo");
	bool needsExport = true;
	if(FPaths::FileExists(exportCheckPath))
	{
		FString jsonStr;
		FFileHelper::LoadFileToString(jsonStr, *exportCheckPath);
		FUianaExport exportData;
		FJsonObjectConverter::JsonObjectStringToUStruct(jsonStr, &exportData);
		needsExport = !exportData.version.Equals(ValorantVersion);
	}
	return needsExport || DevForceReexport;
}

void UUianaImporter::GetObjects(TArray<FString> &actorPaths, TArray<FString> &objPaths, TArray<FString> &matPaths, const TArray<TSharedPtr<FJsonValue>> &jsonArr)
{
	bool skippedBlueprint = false;
	for (TSharedPtr<FJsonValue> component : jsonArr)
	{
		const TSharedPtr<FJsonObject> obj = component.Get()->AsObject();
		if (obj->GetStringField("Type").EndsWith("_C") && obj->HasField("Template"))
		{
			if (!skippedBlueprint)
			{
				skippedBlueprint = true;
				continue;
			}
			actorPaths.AddUnique(obj->GetStringField("Template"));
		}
		if (obj->HasField("Properties"))
		{
			const TSharedPtr<FJsonObject> props = obj->GetObjectField("Properties");
			if (props->HasTypedField<EJson::Object>("StaticMesh"))
			{
				const TSharedPtr<FJsonObject> staticMesh = props->GetObjectField("StaticMesh");
				if (!staticMesh->HasField("ObjectPath")) UE_LOG(LogTemp, Error, TEXT("Uiana: No object path for static mesh %s!"), *obj->GetStringField("Outer"));
				if (!staticMesh->HasField("ObjectPath")) continue;
				const FString path = staticMesh->GetStringField("ObjectPath");
				objPaths.AddUnique( FPaths::Combine(FPaths::GetPath(path), FPaths::GetBaseFilename(path)).Replace(TEXT("/"), TEXT("\\")));
				if (props->HasField("OverrideMaterials"))
				{
					for (TSharedPtr<FJsonValue, ESPMode::ThreadSafe> mat : props->GetArrayField("OverrideMaterials"))
					{
						if (mat->IsNull()) continue;
						if (!mat->AsObject()->HasField("ObjectPath")) UE_LOG(LogTemp, Error, TEXT("Uiana: No object path for override materials!"));
						if (!mat->AsObject()->HasField("ObjectPath")) continue;
						const FString matPath = mat->AsObject()->GetStringField("ObjectPath");
						matPaths.AddUnique(FPaths::Combine(FPaths::GetPath(matPath), FPaths::GetBaseFilename(matPath)).Replace(TEXT("/"), TEXT("\\")));
					}
				}
			}
			else if (props->HasField("DecalMaterial"))
			{
				if (!props->GetObjectField("DecalMaterial")->HasField("ObjectPath")) continue;
				const FString path = props->GetObjectField("DecalMaterial")->GetStringField("ObjectPath");
				matPaths.AddUnique(FPaths::Combine(FPaths::GetPath(path), FPaths::GetBaseFilename(path)).Replace(TEXT("/"), TEXT("\\")));
			}
		}
	}
}

void UUianaImporter::ImportUmap(const TArray<TSharedPtr<FJsonValue>> umapData, const FString umapName)
{
	// Filter objects
	TArray<TSharedPtr<FJsonObject>> filteredObjs;
	TMap<FString, AActor*> bpMapping = {};
	int i = -1;
	for (const TSharedPtr<FJsonValue> objData : umapData)
	{
		i += 1;
		const TSharedPtr<FJsonObject> obj = objData.Get()->AsObject();
		FString objName;
		if (!obj.Get()->HasField("Properties")) objName = "None";
		else if (obj.Get()->GetObjectField("Properties")->HasTypedField<EJson::Object>("StaticMesh"))
		{
			objName = obj.Get()->GetObjectField("Properties")->GetObjectField("StaticMesh")->GetStringField("ObjectPath");
		}
		else if (obj.Get()->HasField("Outer"))
		{
			objName = obj.Get()->GetStringField("Outer");
		}
		else continue;
		objName = FPaths::GetCleanFilename(objName).ToLower();
		if (IsBlacklisted(objName)) continue;
		UianaHelpers::ObjectType objType = UianaHelpers::ParseObjectType(obj->GetStringField("Type"));
		if (Settings->ImportBlueprints && objType == UianaHelpers::ObjectType::Blueprint)
		{
			ImportBlueprint(obj, bpMapping);
		}
		filteredObjs.Add(obj);
	}
	for (const TSharedPtr<FJsonObject> obj : filteredObjs)
	{
		UianaHelpers::ObjectType objType = UianaHelpers::ParseObjectType(obj->GetStringField("Type"));
		if (Settings->ImportMeshes && objType == UianaHelpers::ObjectType::Mesh)
		{
			ImportMesh(obj, umapName, bpMapping);
		}
		if (Settings->ImportDecals && objType == UianaHelpers::ObjectType::Decal)
		{
			ImportDecal(obj);
		}
		if (Settings->ImportLights && objType == UianaHelpers::ObjectType::Light)
		{
			ImportLight(obj);
		}
	}
}

void UUianaImporter::ImportBlueprint(const TSharedPtr<FJsonObject> obj, TMap<FString, AActor*> &bpMapping)
{
	if (!obj->HasField("Properties")) return;
	FTransform transform;
	if (UianaHelpers::HasTransformComponent(obj->GetObjectField("Properties")))
	{
		transform = UianaHelpers::GetTransformComponent(obj->GetObjectField("Properties"));
	}
	else
	{
		transform = UianaHelpers::GetSceneTransformComponent(obj->GetObjectField("Properties"));
	}
	const FString bpName = obj->GetStringField("Type");//.Mid(0, obj->GetStringField("Type").Len() - 2);
	UClass* bpClass = UEditorAssetLibrary::LoadBlueprintClass(FPaths::Combine("/Game/ValorantContent/Blueprints/", bpName + "." + bpName));
	AActor* bpActor = UEditorLevelLibrary::SpawnActorFromObject(bpClass, transform.GetTranslation(), transform.GetRotation().Rotator());
	if (bpActor == nullptr) return;
	bpActor->SetActorLabel(obj->GetStringField("Name"));
	bpActor->SetActorScale3D(transform.GetScale3D());
	bpMapping.Add(bpName, bpActor);
}

void UUianaImporter::FixActorBP(const TSharedPtr<FJsonObject> bpData, const TMap<FString, AActor*> bpMapping)
{
	const FName bpComponentName = FName(*bpData->GetStringField("Name"));
	if (!bpMapping.Contains(bpData->GetStringField("Outer"))) return;
	AActor* bpActor = bpMapping[bpData->GetStringField("Outer")];
	UActorComponent* bpComponent = UBPFL::GetComponentByName(bpActor, bpComponentName);
	if (bpComponent == nullptr) return;
	if (bpData->HasField("StaticMesh"))
	{
		const TSharedPtr<FJsonObject> bpProps = bpData->GetObjectField("Properties");
		FString meshName;
		if (bpProps->TryGetStringField("ObjectName", meshName))
		{
			FString name = meshName.Replace(TEXT("StaticMesh "), TEXT(""), ESearchCase::CaseSensitive);
			UStaticMesh* mesh = static_cast<UStaticMesh*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Meshes/" + name));
			UianaHelpers::SetActorProperty<UStaticMesh*>(UStaticMesh::StaticClass(), bpComponent, "StaticMesh", mesh);
		}
		if (Settings->ImportMaterials && bpProps->HasField("OverrideMaterials"))
		{
			TArray<UMaterialInterface*> overrideMats = CreateOverrideMaterials(bpData);
			if (!overrideMats.IsEmpty() && !bpData->GetStringField("Name").Contains("Barrier"))
			{
				UBPFL::SetOverrideMaterial(bpActor, bpComponentName, overrideMats);
			}
		}
		if (bpProps->HasField("AttachParent") && UianaHelpers::HasTransformComponent(bpData->GetObjectField("Properties")))
		{
			FTransform transform = UianaHelpers::GetTransformComponent(bpProps);
			if (bpProps->HasField("RelativeScale3D")) UianaHelpers::SetActorProperty(UStaticMesh::StaticClass(), bpComponent, "RelativeScale3D", transform.GetScale3D());
			if (bpProps->HasField("RelativeLocation")) UianaHelpers::SetActorProperty(UStaticMesh::StaticClass(), bpComponent, "RelativeLocation", transform.GetTranslation());
			if (bpProps->HasField("RelativeRotation")) UianaHelpers::SetActorProperty(UStaticMesh::StaticClass(), bpComponent, "RelativeRotation", transform.GetRotation().Rotator());
		}
	}
}

void UUianaImporter::ImportMesh(const TSharedPtr<FJsonObject> obj, const FString umapName, const TMap<FString, AActor*> bpMapping)
{
	if (obj->HasField("Template"))
	{
		FixActorBP(obj, bpMapping);
		return;
	}
	if (!obj->GetObjectField("Properties")->HasField("StaticMesh"))
	{
		return;
	}
	FTransform transform = UianaHelpers::GetTransformComponent(obj->GetObjectField("Properties"));
	UClass* meshActorClass = obj->HasField("PerInstanceSMData") && obj->GetStringField("Type").Contains("Instanced") ? AHismActorCPP::StaticClass() : AStaticMeshActor::StaticClass();
	AActor* meshActor = UEditorLevelLibrary::SpawnActorFromClass(meshActorClass, FVector::ZeroVector);
	meshActor->SetActorLabel(obj->GetStringField("Outer"));
	TArray<UObject*> meshActorObjects;
	meshActor->GetDefaultSubobjects(meshActorObjects);
	UHierarchicalInstancedStaticMeshComponent* meshInstancedObject = Cast<UHierarchicalInstancedStaticMeshComponent>(meshActorObjects.Last());
	UStaticMeshComponent* meshObject = Cast<UStaticMeshComponent>(meshActorObjects.Last());
	if (meshInstancedObject != nullptr) // Mesh is Instanced already
	{
		meshActor->SetFolderPath("Meshes/Static");
		const TArray<TSharedPtr<FJsonValue>> perInstanceData = obj->GetArrayField("PerInstanceSMData");
		for (const TSharedPtr<FJsonValue> instance : perInstanceData)
		{
			if (instance->AsObject()->HasField("Properties")) UE_LOG(LogTemp, Error, TEXT("Uiana: Need to use Properties field for PerInstanceSM Data!"));
			FTransform instanceTransform = UianaHelpers::GetTransformComponent(instance->AsObject());
			meshInstancedObject->AddInstance(instanceTransform);
		}
	}
	else
	{
		FString umapType = umapName.Contains("_VFX") ? "VFX" : "Static";
		meshActor->SetFolderPath(FName(*FPaths::Combine("Meshes", umapType)));
	}
	SetBPSettings(obj->GetObjectField("Properties"), meshObject); // TODO: If there are no bugs with this, rename to "SetActorSettings()"!
	meshObject->SetWorldTransform(transform);
	if (obj->HasField("LODData"))
	{
		const TArray<TSharedPtr<FJsonValue>> lodData = obj->GetArrayField("LODData");
		TArray<FColor> vtxArray = {};
		for (const TSharedPtr<FJsonValue> lod : lodData)
		{
			const TSharedPtr<FJsonObject> lodObj = lod->AsObject();
			if (lodObj->HasField("OverrideVertexColors"))
			{
				const TArray<TSharedPtr<FJsonValue>> vertexData = lodObj->GetObjectField("OverrideVertexColors")->GetArrayField("Data");
				for (const TSharedPtr<FJsonValue> color : vertexData)
				{
					vtxArray.Add(UBPFL::ReturnFromHex(color->AsString()));
				}
			}
		}
		FString modelPath = "NoPath";
		if (obj->GetObjectField("Properties")->HasField("StaticMesh"))
		{
			modelPath = FPaths::Combine(ExportAssetsPath.Path, FPaths::GetBaseFilename(obj->GetObjectField("Properties")->GetObjectField("StaticMesh")->GetStringField("ObjectPath"), false)) + ".pskx";
		}
		if (!vtxArray.IsEmpty())
		{
			UBPFL::PaintSMVertices(meshObject, vtxArray, modelPath);
		}
	}
	if (Settings->ImportMaterials && obj->GetObjectField("Properties")->HasField("OverrideMaterials"))
	{
		UianaHelpers::SetActorProperty(UStaticMeshComponent::StaticClass(), meshObject, "OverrideMaterials", CreateOverrideMaterials(obj));
	}
}

void UUianaImporter::ImportDecal(const TSharedPtr<FJsonObject> obj)
{
	if (obj->HasField("Template") || !UianaHelpers::HasTransformComponent(obj->GetObjectField("Properties"))) return;
	FTransform transform = UianaHelpers::GetTransformComponent(obj->GetObjectField("Properties"));
	ADecalActor* decalActor = Cast<ADecalActor>(UEditorLevelLibrary::SpawnActorFromClass(ADecalActor::StaticClass(), transform.GetTranslation(), transform.GetRotation().Rotator()));
	decalActor->SetFolderPath("Decals");
	decalActor->SetActorLabel(obj->GetStringField("Name"));
	decalActor->SetActorScale3D(transform.GetScale3D());
	// FString decalPath = FPaths::GetPath(obj->GetObjectField("DecalMaterial")->GetStringField("ObjectPath"));
	FString decalPath = obj->GetObjectField("Properties")->GetObjectField("DecalMaterial")->GetStringField("ObjectPath");
	UMaterialInstanceConstant* decalMat = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Materials/" + decalPath + "." + decalPath));
	decalActor->SetDecalMaterial(decalMat);
	SetBPSettings(obj->GetObjectField("Properties"), decalActor->GetDecal());
}

void UUianaImporter::ImportLight(const TSharedPtr<FJsonObject> obj)
{
	const FString lightType = obj->GetStringField("Type").Replace(TEXT("Component"), TEXT(""), ESearchCase::CaseSensitive);
	FTransform transform;
	if (UianaHelpers::HasTransformComponent(obj->GetObjectField("Properties")))
	{
		UE_LOG(LogTemp, Display, TEXT("Uiana: Light has TransformComponent, using it"));
		transform = UianaHelpers::GetTransformComponent(obj->GetObjectField("Properties"));
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Uiana: Light is using SceneTransform!"));
		transform = UianaHelpers::GetSceneTransformComponent(obj->GetObjectField("Properties"));
	}
	UClass* lightClass = FEditorClassUtils::GetClassFromString(lightType);
	UEditorActorSubsystem* EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	UE_LOG(LogTemp, Display, TEXT("Uiana: Creating light at position %d, %d, %d"), transform.GetTranslation().X, transform.GetTranslation().Y, transform.GetTranslation().Z);
	AActor* light = EditorActorSubsystem->SpawnActorFromClass(lightClass, transform.GetTranslation(), transform.GetRotation().Rotator());
	light->SetFolderPath(FName("Lights/" + lightType));
	light->SetActorLabel(obj->GetStringField("Name"));
	light->SetActorScale3D(transform.GetScale3D());
	UActorComponent* lightComponent = light->GetRootComponent();
	if (Cast<UBrushComponent>(lightComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Casting BrushComponent's parent as UObject!"));
		lightComponent = Cast<UActorComponent>(light);
		if (lightComponent == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to convert parent of BrushComponent!"));
			return;
		}
		const FProperty* settingsProp = PropertyAccessUtil::FindPropertyByName("Settings", lightComponent->GetClass());
		if (settingsProp != nullptr)
		{
			UianaHelpers::SetActorProperty(lightComponent->GetClass(), lightComponent, "Unbound", true);
			UianaHelpers::SetActorProperty(lightComponent->GetClass(), lightComponent, "Priority", 1.0);
			// UianaHelpers::SetActorProperty(lightComponent->GetClass(), lightComponent, "Unbound", true);
			// UianaHelpers::SetActorProperty(lightComponent->GetClass(), lightComponent, "Priority", 1.0);
			SetBPSettings(obj->GetObjectField("Properties")->GetObjectField("Settings"), lightComponent);
		}
		SetBPSettings(obj->GetObjectField("Properties"), lightComponent);
		return;
	}
	const FProperty* settingsProp = PropertyAccessUtil::FindPropertyByName("Settings", lightComponent->GetClass());
	if (settingsProp != nullptr)
	{
		UianaHelpers::SetActorProperty(lightComponent->GetClass(), lightComponent, "Unbound", true);
		UianaHelpers::SetActorProperty(lightComponent->GetClass(), lightComponent, "Priority", 1.0);
		// UianaHelpers::SetActorProperty(lightComponent->GetClass(), lightComponent, "Unbound", true);
		// UianaHelpers::SetActorProperty(lightComponent->GetClass(), lightComponent, "Priority", 1.0);
		SetBPSettings(obj->GetObjectField("Properties")->GetObjectField("Settings"), lightComponent);
	}
	SetBPSettings(obj->GetObjectField("Properties"), lightComponent);
}

void UUianaImporter::CreateBlueprints(const TArray<FString> bpPaths)
{
	TArray<FString> sceneRoots, childNodes;
	TArray<TSharedPtr<FJsonValue>> newJsons, gameObjs;
	for (const FString bpPath : bpPaths)
	{
		UE_LOG(LogTemp, Display, TEXT("Uiana: Importing BP at path: %s"), *bpPath);
		const TArray<FString> blacklistedBPs = {"SoundBarrier", "SpawnBarrierProjectile", "BP_UnwalkableBlockingVolumeCylinder",
				   "BP_StuckPickupVolume", "BP_BlockingVolume", "TargetingBlockingVolume_Box", "directional_look_up" };
		const FString bpName = FPaths::GetBaseFilename(bpPath);
		if (blacklistedBPs.Contains(bpName)) continue;
		
		// Reduce BP JSON
		FString bpJsonStr;
		FFileHelper::LoadFileToString(bpJsonStr, *bpPath);
		TArray<TSharedPtr<FJsonValue>> bpRaws, bpFiltereds;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(bpJsonStr);
		if (!FJsonSerializer::Deserialize(JsonReader, bpRaws) || !bpRaws[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize blueprint %s"), *bpPath);
			continue;
		}
		for (const TSharedPtr<FJsonValue> bp : bpRaws)
		{
			const TSharedPtr<FJsonObject> bpObj = bp->AsObject();
			if (!bpObj->HasField("Properties")) continue;
			const TSharedPtr<FJsonObject> bpProps = bpObj->GetObjectField("Properties");
			const FString name = bpObj->GetStringField("Name");
			const FString type = bpObj->GetStringField("Type");
			if (type.Equals("SimpleConstructionScript"))
			{
				const FString sceneRoot = bpProps->GetObjectField("DefaultSceneRootNode")->GetStringField("ObjectName");
				FString temp, root;
				sceneRoot.Split(TEXT(":"), &temp, &root, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
				sceneRoots.Add(root);
			}
			if (name.Contains("Node"))
			{
				const FString objName = bpProps->GetObjectField("ComponentTemplate")->GetStringField("ObjectName");
				FString temp, nodeName;
				objName.Split(TEXT(":"), &temp, &nodeName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
				TSharedPtr<FJsonObject> nodeComponent;
				for (const TSharedPtr<FJsonValue> componentBP : bpRaws)
				{
					if (componentBP->AsObject()->HasField("Name"))
					{
						nodeComponent = componentBP->AsObject();
						break;
					}
				}
				if (nodeComponent.IsValid() && nodeComponent->HasField("Properties")) // TODO: CAUSE FOR CRASH - nodeComponent is null and causes crash
					bpProps->SetObjectField("CompProps", nodeComponent->GetObjectField("Properties"));
				newJsons.Add(bp);
				if (bpProps->HasField("ChildNodes"))
				{
					for (const TSharedPtr<FJsonValue> childNode : bpProps->GetArrayField("ChildNodes"))
					{
						FString temp1, childName;
						const FString childNameRaw = childNode.Get()->AsObject()->GetStringField("ObjectName");
						childNameRaw.Split(TEXT("."), &temp1, &childName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
						childNodes.Add(childName);
					}
				}
			}
			if (name.Equals("GameObjectMesh"))
			{
				gameObjs.Add(bp);
			}
		}
		// FJsonObject bpJson = FJsonObject();
		// bpJson.SetArrayField("Nodes", newJsons);
		// bpJson.SetArrayField("SceneRoot", sceneRoots);
		// bpJson.SetArrayField("GameObjects", gameObjs);
		// bpJson.SetArrayField("ChildNodes", childNodes);

		// Create BP
		UE_LOG(LogTemp, Display, TEXT("Uiana: Checking if BP %s exists"), *bpName);
		UBlueprint* bpActor = Cast<UBlueprint>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Blueprints/" + bpName));
		if (bpActor != nullptr) continue;
		UE_LOG(LogTemp, Display, TEXT("Uiana: Creating BP %s"), *bpName);
		UBlueprintFactory* factory = NewObject<UBlueprintFactory>();
		IAssetTools& assetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		bpActor = static_cast<UBlueprint*>(assetTools.CreateAsset(bpName, "/Game/ValorantContent/Blueprints/", UBlueprint::StaticClass(), factory));
		
		if (newJsons.IsEmpty())
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: No nodes in BP %s, skipping"), *bpName);
			continue;
		}
		// Get all characters after '.' using GetExtension. Little hacky but works
		const FString defaultRoot = FPaths::GetExtension(sceneRoots[0]);
		int defaultIndex = newJsons.IndexOfByKey(defaultRoot);
		TSharedPtr<FJsonValue> temp = newJsons[defaultIndex];
		newJsons.RemoveAt(defaultIndex, 1, false);
		newJsons.Push(temp);
		TArray<USCS_Node*> bpNodeArray;
		for (int i = newJsons.Num() - 1; i >= 0; i--)
		{
			const TSharedPtr<FJsonValue> bpNode = newJsons[i];
			const TSharedPtr<FJsonObject> bpNodeObj = bpNode.Get()->AsObject();
			const FString bpNodeName = bpNodeObj->GetStringField("Name");
			TSharedPtr<FJsonObject> bpNodeProps = bpNodeObj->GetObjectField("Properties");
			if (childNodes.Contains(bpNodeName)) continue;
			UE_LOG(LogTemp, Display, TEXT("Uiana: Importing BP Node %s"), *bpNodeObj->GetStringField("Name"));
			const FString componentName = bpNodeProps->GetObjectField("ComponentClass")->GetStringField("ObjectName").Replace(TEXT("Class "), TEXT(""), ESearchCase::CaseSensitive);
			UClass* componentClass = FEditorClassUtils::GetClassFromString(componentName);
			if (componentClass == nullptr)
			{
				UE_LOG(LogTemp, Display, TEXT("Uiana: Node Class %s not Unreal Component, skipping"), *componentName);
				continue;
			}
			if (bpNodeProps->HasField("ChildNodes"))
			{
				UE_LOG(LogTemp, Display, TEXT("Uiana: Importing Child Nodes for BP Node %s"), *bpNodeName);
				bpNodeArray = GetLocalBPChildren(bpNodeProps->GetArrayField("ChildNodes"), newJsons, bpActor);
			}
			const FString componentInternalName = bpNodeProps->GetStringField("InternalVariableName");
			UActorComponent* component = UBPFL::CreateBPComp(bpActor, componentClass, FName(*componentInternalName), bpNodeArray);
			if (bpNodeProps->HasField("CompProps"))
			{
				UE_LOG(LogTemp, Display, TEXT("Uiana: BP Node %s has specified component properties, using them."), *bpNodeName);
				bpNodeProps = bpNodeProps->GetObjectField("CompProps");
			}
			UE_LOG(LogTemp, Display, TEXT("Uiana: Setting BP Node %s properties!"), *bpNodeName);
			SetBPSettings(bpNodeProps, component);
			// TODO: Rework this SetBPSettings() + Set editor property section, corresponds with set_mesh_settings and handle_child_nodes
			FTransform transform = UianaHelpers::GetTransformComponent(bpNodeProps);
			UianaHelpers::SetActorProperty<FVector>(componentClass, component, "RelativeLocation", transform.GetLocation());
			// FObjectEditorUtils::SetPropertyValue(componentNode, FName("RelativeLocation"), transform.GetLocation());
			UianaHelpers::SetActorProperty<FRotator>(componentClass, component, "RelativeRotation", transform.GetRotation().Rotator());
			// FObjectEditorUtils::SetPropertyValue(componentNode, FName("RelativeRotation"), transform.GetRotation());
			UianaHelpers::SetActorProperty<FVector>(componentClass, component, "RelativeScale3D", transform.GetScale3D());
			// FObjectEditorUtils::SetPropertyValue(componentNode, FName("RelativeScale3D"), transform.GetScale3D());
		}
		if (bpName.Equals("SpawnBarrier")) continue;
		for (TSharedPtr<FJsonValue> gameObj : gameObjs)
		{
			UClass* componentClass = UStaticMeshComponent::StaticClass();
			UActorComponent* component = UBPFL::CreateBPComp(bpActor, componentClass, FName("GameObjectMesh"), bpNodeArray);
			const TSharedPtr<FJsonObject> gameObjProps = gameObj->AsObject()->GetObjectField("Properties");
			SetBPSettings(gameObjProps, component);
			FTransform transform = UianaHelpers::GetTransformComponent(gameObjProps);
			// TODO: Make this function with above!
			UianaHelpers::SetActorProperty<FVector>(componentClass, component, "RelativeLocation", transform.GetLocation());
			// FObjectEditorUtils::SetPropertyValue(componentNode, FName("RelativeLocation"), transform.GetLocation());
			// FObjectEditorUtils::SetPropertyValue(componentNode, FName("RelativeRotation"), transform.GetRotation());
			UianaHelpers::SetActorProperty<FVector>(componentClass, component, "RelativeScale3D", transform.GetScale3D());
			// FObjectEditorUtils::SetPropertyValue(componentNode, FName("RelativeScale3D"), transform.GetScale3D());
			UianaHelpers::SetActorProperty<FRotator>(componentClass, component, "RelativeRotation", transform.GetRotation().Rotator());
		}
	}
}

TArray<USCS_Node*> UUianaImporter::GetLocalBPChildren(TArray<TSharedPtr<FJsonValue>> childNodes, TArray<TSharedPtr<FJsonValue>> bpData, UBlueprint* bpActor)
{
	TArray<USCS_Node*> localChildren;
	for (const TSharedPtr<FJsonValue> childNode : childNodes)
	{
		const TSharedPtr<FJsonObject> childObj = childNode->AsObject();
		const FString childName = childObj->GetStringField("ObjectName");
		for (const TSharedPtr<FJsonValue> cNode : bpData)
		{
			const TSharedPtr<FJsonObject> nodeDataObj = cNode->AsObject();
			// FString temp, componentName;
			// nodeDataObj->GetObjectField("Properties")->GetObjectField("ComponentClass")->GetStringField("ObjectName").Split(TEXT("."), &temp, &componentName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			const FString componentName = nodeDataObj->GetObjectField("Properties")->GetObjectField("ComponentClass")->GetStringField("ObjectName").Replace(TEXT("Class "), TEXT(""), ESearchCase::CaseSensitive);
			UClass* componentClass = FEditorClassUtils::GetClassFromString(componentName);
			if (componentClass == nullptr) continue;
			const FString internalName = nodeDataObj->GetObjectField("Properties")->GetStringField("InternalVariableName");
			if (internalName.Contains("TargetViewMode") || internalName.Contains("Decal1") || internalName.Contains("SM_Barrier_Back_VisionBlocker")) continue;

			if (nodeDataObj->GetStringField("Name").Equals(childName))
			{
				UActorComponent* componentNode;
				USCS_Node* unrealNode = UBPFL::CreateNode(bpActor, componentClass, FName(*internalName), componentNode);
				SetBPSettings(nodeDataObj->GetObjectField("Properties")->GetObjectField("CompProps"), componentNode);
				if (UianaHelpers::HasTransformComponent(nodeDataObj->GetObjectField("Properties")->GetObjectField("CompProps")))
				{
					FTransform transform = UianaHelpers::GetTransformComponent(nodeDataObj->GetObjectField("Properties")->GetObjectField("CompProps"));
					UianaHelpers::SetActorProperty<FTransform>(UActorComponent::StaticClass(), componentNode, "RelativeLocation", transform);
					// FObjectEditorUtils::SetPropertyValue(componentNode, FName("RelativeLocation"), transform.GetLocation());
					UianaHelpers::SetActorProperty<FTransform>(UActorComponent::StaticClass(), componentNode, "RelativeRotation", transform);
					// FObjectEditorUtils::SetPropertyValue(componentNode, FName("RelativeRotation"), transform.GetRotation());
					UianaHelpers::SetActorProperty<FTransform>(UActorComponent::StaticClass(), componentNode, "RelativeScale3D", transform);
					// FObjectEditorUtils::SetPropertyValue(componentNode, FName("RelativeScale3D"), transform.GetScale3D());
				}
				break;
			}
		}
	}
	return localChildren;
}

void UUianaImporter::SetObjSettings(const TSharedPtr<FJsonObject> bpProps, UObject* mainObj)
{
	// Loop through all JSON values (type <FString, TSharedPtr<FJsonValue>>)
	for (auto const& prop : bpProps.Get()->Values)
	{
		TSharedPtr<FJsonValue> propValue = prop.Value;
		const FName propName = FName(*prop.Key);
		FProperty* objectProp = PropertyAccessUtil::FindPropertyByName(propName, mainObj->GetClass());
		if (objectProp == nullptr) continue;
		const EJson propType = propValue.Get()->Type;
		if (propType == EJson::Number || propType == EJson::Boolean)
		{
			if (prop.Key.Equals("InfluenceRadius") && propValue.Get()->AsNumber() == 0)
			{
				// UianaHelpers::SetActorProperty<float>(mainObj->GetClass(), mainObj, prop.Key, 14680.0);
				// FObjectEditorUtils::SetPropertyValue(mainObj, propName, 14680);
				UianaHelpers::SetGivenObjectProperty(mainObj, objectProp, 14680);
				continue;
			}
			// UianaHelpers::SetActorProperty<float>(mainObj->GetClass(), mainObj, prop.Key, prop.Value.Get()->AsNumber());
			UianaHelpers::SetGivenObjectProperty(mainObj, objectProp, prop.Value.Get()->AsNumber());
			// FObjectEditorUtils::SetPropertyValue(mainObj, propName, prop.Value.Get()->AsNumber());
			continue;
		}
		// TODO: This is necessary for lighting/post processing stuff but crashes BP due to needing to free the propValue afterward
		// if (propType == EJson::String && propValue.Get()->AsString().Contains("::"))
		// {
		// 	FString temp, splitResult;
		// 	propValue.Get()->AsString().Split("::", &temp, &splitResult);
		// 	FJsonValueString valString = FJsonValueString(splitResult);
		// 	propValue = MakeShareable<FJsonValueString>(&valString);
		// }
		if (objectProp->GetClass()->GetName().Equals("FLinearColor"))
		{
			FLinearColor color;
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			color.R = obj->GetNumberField("R");
			color.G = obj->GetNumberField("G");
			color.B = obj->GetNumberField("B");
			UianaHelpers::SetGivenObjectProperty(mainObj, objectProp, color);
			// UianaHelpers::SetActorProperty<FLinearColor>(mainObj->GetClass(), mainObj, prop.Key, color);
			// FObjectEditorUtils::SetPropertyValue(mainObj, propName, color);
		}
		else if (objectProp->GetClass()->GetName().Equals("FVector4"))
		{
			FVector4 vector;
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			vector.X = obj->GetNumberField("X");
			vector.Y = obj->GetNumberField("Y");
			vector.Z = obj->GetNumberField("Z");
			vector.W = obj->GetNumberField("W");
			UianaHelpers::SetGivenObjectProperty(mainObj, objectProp, vector);
			// UianaHelpers::SetActorProperty<FVector4>(mainObj->GetClass(), mainObj, prop.Key, vector);
			// FObjectEditorUtils::SetPropertyValue(mainObj, propName, vector);
		}
		else if (objectProp->GetClass()->GetName().Contains("Color"))
		{
			FColor color;
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			color.R = obj->GetNumberField("R");
			color.G = obj->GetNumberField("G");
			color.B = obj->GetNumberField("B");
			color.A = obj->GetNumberField("A");
			UianaHelpers::SetGivenObjectProperty(mainObj, objectProp, color);
			// UianaHelpers::SetActorProperty<FColor>(mainObj->GetClass(), mainObj, prop.Key, color);
			// FObjectEditorUtils::SetPropertyValue(mainObj, propName, color);
		}
		else if (propType == EJson::Object)
		{
			if (prop.Key.Equals("IESTexture"))
			{
				FString temp, newTextureName;
				propValue.Get()->AsObject()->GetStringField("ObjectName").Split("_", &temp, &newTextureName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
				FString assetPath = "/UianaCPP/IESProfiles/" + newTextureName + "." + newTextureName;
				UTexture* newTexture = static_cast<UTexture*>(UEditorAssetLibrary::LoadAsset(assetPath));
				UianaHelpers::SetGivenObjectProperty(mainObj, objectProp, newTexture);
				// UianaHelpers::SetActorProperty<UTexture*>(mainObj->GetClass(), mainObj, prop.Key, newTexture);
				// FObjectEditorUtils::SetPropertyValue(mainObj, propName, newTexture);
			}
			else if (prop.Key.Equals("Cubemap"))
			{
				FString newCubemapName = propValue.Get()->AsObject()->GetStringField("ObjectName").Replace(TEXT("TextureCube "), TEXT(""));
				FString assetPath = "/UianaCPP/CubeMaps/" + newCubemapName + "." + newCubemapName;
				// TODO: Convert all static_cast with UObjects to Cast<>()
				UTextureCube* newCube = Cast<UTextureCube, UObject>(UEditorAssetLibrary::LoadAsset(assetPath));
				UianaHelpers::SetGivenObjectProperty(mainObj, objectProp, newCube);
				// UianaHelpers::SetActorProperty<UTextureCube*>(mainObj->GetClass(), mainObj, prop.Key, newCube);
				// FObjectEditorUtils::SetPropertyValue(mainObj, propName, newCube);
			}
			else if (prop.Key.Equals("DecalMaterial"))
			{
				// UE_LOG(LogTemp, Warning, TEXT("Uiana: Need to set BP Property %s somehow!"), *prop.Key);
				FString decalPath = FPaths::GetPath(propValue.Get()->AsObject()->GetStringField("ObjectPath"));
				UMaterialInstanceConstant* decalMat = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Materials/" + decalPath + "." + decalPath));
				UDecalComponent decalComponent;
				decalComponent.SetMaterial(0, Cast<UMaterialInterface>(mainObj));
				decalComponent.SetDecalMaterial(decalMat);
			}
			else if (prop.Key.Equals("DecalSize"))
			{
				FVector vec;
				const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
				vec.X = obj->GetNumberField("X");
				vec.Y = obj->GetNumberField("Y");
				vec.Z = obj->GetNumberField("Z");
				UianaHelpers::SetGivenObjectProperty(mainObj, objectProp, vec);
				// UianaHelpers::SetActorProperty<FVector>(mainObj->GetClass(), mainObj, prop.Key, vec);
				// FObjectEditorUtils::SetPropertyValue(mainObj, propName, vec);
			}
			else if (prop.Key.Equals("StaticMesh"))
			{
				FString meshName;
				if (propValue->AsObject()->TryGetStringField("ObjectName", meshName))
				{
					FString name = meshName.Replace(TEXT("StaticMesh "), TEXT(""), ESearchCase::CaseSensitive);
					UStaticMesh* mesh = static_cast<UStaticMesh*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Meshes/" + name));
					if (mesh == nullptr) continue;
					UianaHelpers::SetGivenObjectProperty(mainObj, objectProp, mesh);
					// UianaHelpers::SetActorProperty<UStaticMesh*>(mainObj->GetClass(), mainObj, prop.Key, mesh);
					// FObjectEditorUtils::SetPropertyValue(mainObj, propName, mesh);
				}
			}
			else if (prop.Key.Equals("BoxExtent"))
			{
				FVector vec;
				const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
				vec.X = obj->GetNumberField("X");
				vec.Y = obj->GetNumberField("Y");
				vec.Z = obj->GetNumberField("Z");
				// UianaHelpers::SetActorProperty<FVector>(mainObj->GetClass(), mainObj, "BoxExtent", vec);
				// FObjectEditorUtils::SetPropertyValue(mainObj, "BoxExtent", vec);
				UianaHelpers::SetGivenObjectProperty(mainObj, objectProp, vec);
			}
			else if (prop.Key.Equals("LightmassSettings"))
			{
				// TODO: Investigate why lightmass settings do not seem to be getting applied although no error!
				FLightmassMaterialInterfaceSettings lightmassSettings;
				FJsonObjectConverter::JsonObjectToUStruct(propValue.Get()->AsObject().ToSharedRef(), &lightmassSettings);
				// if (!UianaHelpers::SetActorProperty<FLightmassMaterialInterfaceSettings>(mainObj->GetClass(), mainObj, "LightmassSettings", lightmassSettings)) UE_LOG(LogTemp, Error, TEXT("Uiana: Failed to set lightmass settings for BP!"));
				if (!UianaHelpers::SetGivenObjectProperty(mainObj, objectProp, lightmassSettings)) UE_LOG(LogTemp, Error, TEXT("Uiana: Failed to set lightmass settings!"));
			}
		}
		else if (propType == EJson::Array && prop.Key.Equals("OverrideMaterials"))
		{
			TArray<UMaterialInstanceConstant*> overrideMats = {};
			for (TSharedPtr<FJsonValue, ESPMode::ThreadSafe> ovrMat : propValue.Get()->AsArray())
			{
				if (!ovrMat.IsValid() || ovrMat.Get()->IsNull())
				{
					overrideMats.Add(nullptr);
					continue;
				}
				const TSharedPtr<FJsonObject> matData = ovrMat.Get()->AsObject();
				FString temp, objName;
				matData.Get()->GetStringField("ObjectName").Split(" ", &temp, &objName, ESearchCase::Type::IgnoreCase, ESearchDir::FromEnd);
				if (matData->HasTypedField<EJson::Object>("ObjectName")) UE_LOG(LogTemp, Error, TEXT("Uiana: Obj Override Material ObjectName is not a string!"));
				if (objName.Contains("MaterialInstanceDynamic")) continue;
				UMaterialInstanceConstant* loaded = nullptr;
				loaded = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset(FPaths::Combine("/UianaCPP/Materials/", objName)));
				if (loaded == nullptr) loaded = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset(FPaths::Combine("/Game/ValorantContent/Materials/", objName)));
				if (loaded == nullptr)
				{
					UE_LOG(LogTemp, Display, TEXT("Failed to load override material %s, skipping"), *objName);
					continue;
				}
				overrideMats.Add(loaded);
			}
			// UianaHelpers::SetActorProperty<TArray<UMaterialInstanceConstant*>>(mainObj->GetClass(), mainObj, prop.Key, overrideMats);
			// FObjectEditorUtils::SetPropertyValue(mainObj, propName, overrideMats);
			UianaHelpers::SetGivenObjectProperty(mainObj, objectProp, overrideMats);
		}
		else
		{
			if (propType == EJson::Array)
			{
				const TArray<TSharedPtr<FJsonValue>> parameterArray = propValue.Get()->AsArray();
				if (prop.Key.Equals("TextureStreamingData"))
				{
					TArray<FMaterialTextureInfo> textures;
					for (const TSharedPtr<FJsonValue> texture : parameterArray)
					{
						FMaterialTextureInfo textureInfo;
						const TSharedPtr<FJsonObject> textureObj = texture.Get()->AsObject();
						FJsonObjectConverter::JsonObjectToUStruct(textureObj.ToSharedRef(), &textureInfo);
						textures.Add(textureInfo);
					}
					UE_LOG(LogTemp, Warning, TEXT("Uiana: Need to set TextureStreamingData for Obj!"));
					// bp->SetTextureStreamingData(textures);
				}
				else
				{
					if (prop.Key.Equals("ScalarParameterValues") || prop.Key.Equals("VectorParameterValues") || prop.Key.Equals("TextureParameterValues")) continue;
					for (const TSharedPtr<FJsonValue> parameter : parameterArray)
					{
						const TSharedPtr<FJsonObject> paramObj = parameter.Get()->AsObject();
						// const TSharedPtr<FJsonObject> paramInfo = paramObj.Get()->GetObjectField("ParameterInfo");
						// FMaterialParameterInfo info;
						// FJsonObjectConverter::JsonObjectToUStruct(paramInfo.ToSharedRef(), &info);
						// if (prop.Key.Equals("ScalarParameterValues"))
						// {
						// 	double paramValue = paramObj.Get()->GetNumberField("ParameterValue");
						// 	mat->SetScalarParameterValueEditorOnly(info, paramValue);
						// }
						// else if (prop.Key.Equals("VectorParameterValues"))
						// {
						// 	FLinearColor paramValue;
						// 	const TSharedPtr<FJsonObject> vectorParams = paramObj.Get()->GetObjectField("ParameterValue");
						// 	paramValue.R = vectorParams.Get()->GetNumberField("R");
						// 	paramValue.G = vectorParams.Get()->GetNumberField("G");
						// 	paramValue.B = vectorParams.Get()->GetNumberField("B");
						// 	paramValue.A = vectorParams.Get()->GetNumberField("A");
						// 	mat->SetVectorParameterValueEditorOnly(info, paramValue);
						// }
						// else if (prop.Key.Equals("TextureParameterValues"))
						// {
						// 	FString temp, textureName;
						// 	paramObj.Get()->GetObjectField("ParameterValue").Get()->GetStringField("ObjectName").Split(TEXT(" "), &temp, &textureName);
						// 	UTexture* paramValue = static_cast<UTexture*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Textures/" + textureName));
						// 	mat->SetTextureParameterValueEditorOnly(info, paramValue);
						// }
						// else
						// {
						FString OutputString;
						TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
						FJsonSerializer::Serialize(paramObj.ToSharedRef(), Writer);
						UE_LOG(LogTemp, Warning, TEXT("Uiana: Array Obj Property %s unaccounted"), *prop.Key);	
						// }
					}	
				}
			}
			else UE_LOG(LogTemp, Warning, TEXT("Uiana: Need to set Obj Property %s of unknown type!"), *prop.Key);
		}
	}
}

void UUianaImporter::SetBPSettings(const TSharedPtr<FJsonObject> bpProps, UActorComponent* bp)
{
	// Loop through all JSON values (type <FString, TSharedPtr<FJsonValue>>)
	for (auto const& prop : bpProps.Get()->Values)
	{
		TSharedPtr<FJsonValue> propValue = prop.Value;
		const FName propName = FName(*prop.Key);
		const FProperty* objectProp = PropertyAccessUtil::FindPropertyByName(propName, bp->GetClass());
		if (objectProp == nullptr) continue;
		const EJson propType = propValue.Get()->Type;
		if (propType == EJson::Number || propType == EJson::Boolean)
		{
			if (prop.Key.Equals("InfluenceRadius") && propValue.Get()->AsNumber() == 0)
			{
				UianaHelpers::SetActorProperty<float>(bp->GetClass(), bp, prop.Key, 14680.0);
				// FObjectEditorUtils::SetPropertyValue(bp, propName, 14680);
				continue;
			}
			UianaHelpers::SetActorProperty<float>(bp->GetClass(), bp, prop.Key, prop.Value.Get()->AsNumber());
			// FObjectEditorUtils::SetPropertyValue(bp, propName, prop.Value.Get()->AsNumber());
			continue;
		}
		if (objectProp->GetClass()->GetName().Equals("FLinearColor"))
		{
			FLinearColor color;
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			color.R = obj->GetNumberField("R");
			color.G = obj->GetNumberField("G");
			color.B = obj->GetNumberField("B");
			UianaHelpers::SetActorProperty<FLinearColor>(bp->GetClass(), bp, prop.Key, color);
			// FObjectEditorUtils::SetPropertyValue(bp, propName, color);
		}
		else if (objectProp->GetClass()->GetName().Equals("FVector4"))
		{
			FVector4 vector;
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			vector.X = obj->GetNumberField("X");
			vector.Y = obj->GetNumberField("Y");
			vector.Z = obj->GetNumberField("Z");
			vector.W = obj->GetNumberField("W");
			UianaHelpers::SetActorProperty<FVector4>(bp->GetClass(), bp, prop.Key, vector);
			// FObjectEditorUtils::SetPropertyValue(bp, propName, vector);
		}
		else if (objectProp->GetClass()->GetName().Contains("Color"))
		{
			FColor color;
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			color.R = obj->GetNumberField("R");
			color.G = obj->GetNumberField("G");
			color.B = obj->GetNumberField("B");
			color.A = obj->GetNumberField("A");
			UianaHelpers::SetActorProperty<FColor>(bp->GetClass(), bp, prop.Key, color);
			// FObjectEditorUtils::SetPropertyValue(bp, propName, color);
		}
		else if (propType == EJson::Object)
		{
			if (prop.Key.Equals("IESTexture"))
			{
				FString temp, newTextureName;
				propValue.Get()->AsObject()->GetStringField("ObjectName").Split("_", &temp, &newTextureName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
				FString assetPath = "/UianaCPP/IESProfiles/" + newTextureName + "." + newTextureName;
				UTexture* newTexture = static_cast<UTexture*>(UEditorAssetLibrary::LoadAsset(assetPath));
				UianaHelpers::SetActorProperty<UTexture*>(bp->GetClass(), bp, prop.Key, newTexture);
				// FObjectEditorUtils::SetPropertyValue(bp, propName, newTexture);
			}
			else if (prop.Key.Equals("Cubemap"))
			{
				FString newCubemapName = propValue.Get()->AsObject()->GetStringField("ObjectName").Replace(TEXT("TextureCube "), TEXT(""));
				FString assetPath = "/UianaCPP/CubeMaps/" + newCubemapName + "." + newCubemapName;
				// TODO: Convert all static_cast with UObjects to Cast<>()
				UTextureCube* newCube = Cast<UTextureCube, UObject>(UEditorAssetLibrary::LoadAsset(assetPath));
				UianaHelpers::SetActorProperty<UTextureCube*>(bp->GetClass(), bp, prop.Key, newCube);
				// FObjectEditorUtils::SetPropertyValue(bp, propName, newCube);
			}
			else if (prop.Key.Equals("DecalSize"))
			{
				FVector vec;
				const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
				vec.X = obj->GetNumberField("X");
				vec.Y = obj->GetNumberField("Y");
				vec.Z = obj->GetNumberField("Z");
				UianaHelpers::SetActorProperty<FVector>(bp->GetClass(), bp, prop.Key, vec);
				// FObjectEditorUtils::SetPropertyValue(bp, propName, vec);
			}
			else if (prop.Key.Equals("StaticMesh"))
			{
				FString meshName;
				if (propValue->AsObject()->TryGetStringField("ObjectName", meshName))
				{
					FString name = meshName.Replace(TEXT("StaticMesh "), TEXT(""), ESearchCase::CaseSensitive);
					UStaticMesh* mesh = static_cast<UStaticMesh*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Meshes/" + name));
					if (mesh == nullptr) continue;
					UianaHelpers::SetActorProperty<UStaticMesh*>(bp->GetClass(), bp, prop.Key, mesh);
					// FObjectEditorUtils::SetPropertyValue(bp, propName, mesh);
				}
			}
			else if (prop.Key.Equals("BoxExtent"))
			{
				FVector vec;
				const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
				vec.X = obj->GetNumberField("X");
				vec.Y = obj->GetNumberField("Y");
				vec.Z = obj->GetNumberField("Z");
				UianaHelpers::SetActorProperty<FVector>(bp->GetClass(), bp, "BoxExtent", vec);
				// FObjectEditorUtils::SetPropertyValue(bp, "BoxExtent", vec);
			}
			else if (prop.Key.Equals("LightmassSettings"))
			{
				// TODO: Investigate why lightmass settings do not seem to be getting applied although no error!
				FLightmassMaterialInterfaceSettings lightmassSettings;
				FJsonObjectConverter::JsonObjectToUStruct(propValue.Get()->AsObject().ToSharedRef(), &lightmassSettings);
				if (!UianaHelpers::SetActorProperty<FLightmassMaterialInterfaceSettings>(bp->GetClass(), bp, "LightmassSettings", lightmassSettings)) UE_LOG(LogTemp, Error, TEXT("Uiana: Failed to set lightmass settings for BP!"));
				// if (!FObjectEditorUtils::SetPropertyValue(bp, "LightmassSettings", lightmassSettings)) UE_LOG(LogTemp, Error, TEXT("Uiana: Failed to set lightmass settings!"));
			}
		}
		else if (propType == EJson::Array && prop.Key.Equals("OverrideMaterials"))
		{
			TArray<UMaterialInstanceConstant*> overrideMats = {};
			for (TSharedPtr<FJsonValue, ESPMode::ThreadSafe> ovrMat : propValue.Get()->AsArray())
			{
				if (!ovrMat.IsValid() || ovrMat.Get()->IsNull())
				{
					overrideMats.Add(nullptr);
				 	continue;
				}
				if (ovrMat.Get()->Type != EJson::Object)
				{
					FString OutputString;
					TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
					TArray<TSharedPtr<FJsonValue>> TempArr = {ovrMat};
					FJsonSerializer::Serialize(TempArr, Writer);
					UE_LOG(LogTemp, Error, TEXT("Uiana: BP Override Material in Array is not an object and instead is:\n %s!"), *OutputString);
					overrideMats.Add(nullptr);
					continue;
				}
				const TSharedPtr<FJsonObject> matData = ovrMat.Get()->AsObject();
				FString temp, objName;
				matData->GetStringField("ObjectName").Split(" ", &temp, &objName, ESearchCase::Type::IgnoreCase, ESearchDir::FromEnd);
				if (matData->HasTypedField<EJson::Object>("ObjectName")) UE_LOG(LogTemp, Error, TEXT("Uiana: BP Override Material ObjectName is not a string!"));
				if (objName.Contains("MaterialInstanceDynamic")) continue;
				UMaterialInstanceConstant* loaded = nullptr;
				loaded = Cast<UMaterialInstanceConstant>(UEditorAssetLibrary::LoadAsset(FPaths::Combine("/UianaCPP/Materials/", objName)));
				if (loaded == nullptr) loaded = Cast<UMaterialInstanceConstant>(UEditorAssetLibrary::LoadAsset(FPaths::Combine("/Game/ValorantContent/Materials/", objName)));
				if (loaded == nullptr)
				{
					UE_LOG(LogTemp, Display, TEXT("Failed to load override material %s, skipping"), *objName);
					continue;
				}
				overrideMats.Add(loaded);
			}
			UianaHelpers::SetActorProperty<TArray<UMaterialInstanceConstant*>>(bp->GetClass(), bp, prop.Key, overrideMats);
			// FObjectEditorUtils::SetPropertyValue(bp, propName, overrideMats);
		}
		else if (propType == EJson::String)
		{
			if (prop.Key.Equals("Mobility"))
			{
				UianaHelpers::SetActorProperty(bp->GetClass(), bp, prop.Key, UianaHelpers::ParseMobility(propValue->AsString()));
			}
			else if (prop.Key.Equals("DetailMode"))
			{
				UianaHelpers::SetActorProperty(bp->GetClass(), bp, prop.Key, UianaHelpers::ParseDetailMode(propValue->AsString()));
			}
		}
		else
		{
			if (propType == EJson::Array)
			{
				const TArray<TSharedPtr<FJsonValue>> parameterArray = propValue.Get()->AsArray();
				if (prop.Key.Equals("TextureStreamingData"))
				{
					TArray<FMaterialTextureInfo> textures;
					for (const TSharedPtr<FJsonValue> texture : parameterArray)
					{
						FMaterialTextureInfo textureInfo;
						const TSharedPtr<FJsonObject> textureObj = texture.Get()->AsObject();
						FJsonObjectConverter::JsonObjectToUStruct(textureObj.ToSharedRef(), &textureInfo);
						textures.Add(textureInfo);
					}
					UE_LOG(LogTemp, Warning, TEXT("Uiana: Need to set TextureStreamingData for BP!"));
					// bp->SetTextureStreamingData(textures);
				}
				else
				{
					if (prop.Key.Equals("ScalarParameterValues") || prop.Key.Equals("VectorParameterValues") || prop.Key.Equals("TextureParameterValues")) continue;
					for (const TSharedPtr<FJsonValue> parameter : parameterArray)
					{
						const TSharedPtr<FJsonObject> paramObj = parameter.Get()->AsObject();
						// const TSharedPtr<FJsonObject> paramInfo = paramObj.Get()->GetObjectField("ParameterInfo");
						// FMaterialParameterInfo info;
						// FJsonObjectConverter::JsonObjectToUStruct(paramInfo.ToSharedRef(), &info);
						// if (prop.Key.Equals("ScalarParameterValues"))
						// {
						// 	double paramValue = paramObj.Get()->GetNumberField("ParameterValue");
						// 	mat->SetScalarParameterValueEditorOnly(info, paramValue);
						// }
						// else if (prop.Key.Equals("VectorParameterValues"))
						// {
						// 	FLinearColor paramValue;
						// 	const TSharedPtr<FJsonObject> vectorParams = paramObj.Get()->GetObjectField("ParameterValue");
						// 	paramValue.R = vectorParams.Get()->GetNumberField("R");
						// 	paramValue.G = vectorParams.Get()->GetNumberField("G");
						// 	paramValue.B = vectorParams.Get()->GetNumberField("B");
						// 	paramValue.A = vectorParams.Get()->GetNumberField("A");
						// 	mat->SetVectorParameterValueEditorOnly(info, paramValue);
						// }
						// else if (prop.Key.Equals("TextureParameterValues"))
						// {
						// 	FString temp, textureName;
						// 	paramObj.Get()->GetObjectField("ParameterValue").Get()->GetStringField("ObjectName").Split(TEXT(" "), &temp, &textureName);
						// 	UTexture* paramValue = static_cast<UTexture*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Textures/" + textureName));
						// 	mat->SetTextureParameterValueEditorOnly(info, paramValue);
						// }
						// else
						// {
						FString OutputString;
						TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
						FJsonSerializer::Serialize(paramObj.ToSharedRef(), Writer);
						UE_LOG(LogTemp, Warning, TEXT("Uiana: Array BP Property %s unaccounted for"), *prop.Key);
						// }
					}	
				}
			}
			else
			{
				FString type = "None";
				if (propType == EJson::Array) type = "Array";
				if (propType == EJson::Object) type = "Object";
				if (propType == EJson::Boolean) type = "Bool";
				if (propType == EJson::String) type = "String";
				if (propType == EJson::Number) type = "Number";
				if (propType == EJson::Null) type = "Null";
				UE_LOG(LogTemp, Warning, TEXT("Uiana: Need to set BP Property %s of type %s!"), *prop.Key, *type);
				if (propType == EJson::String)
				{
					UE_LOG(LogTemp, Warning, TEXT("Uiana: String BP Property unset - %s: %s"), *prop.Key, *propValue->AsString());
				}
			}
		}
	}
}

bool UUianaImporter::IsBlacklisted(const FString itemName)
{
	for (const FString blacklistObj : BlacklistedObjs)
	{
		if (itemName.Contains(blacklistObj)) return true;
	}
	return false;
}

TArray<UMaterialInterface*> UUianaImporter::CreateOverrideMaterials(const TSharedPtr<FJsonObject> obj)
{
	TArray<UMaterialInterface*> mats = {};
	for (const TSharedPtr<FJsonValue> mat : obj->GetObjectField("Properties")->GetArrayField("OverrideMaterials"))
	{
		if (mat.Get()->Type != EJson::Object)
		{
		    FString OutputString;
		    TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
		    TArray<TSharedPtr<FJsonValue>> TempArr = {mat};
		    FJsonSerializer::Serialize(TempArr, Writer);
		    UE_LOG(LogTemp, Error, TEXT("Uiana: Override Material in Array is not Object and instead is:\n %s!"), *OutputString);
		    continue;
		}
		FString objName, temp;
		mat->AsObject()->GetStringField("ObjectName").Split(TEXT(" "), &temp, &objName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		if (objName.Equals("Stone_M2_Steps_MI1")) objName = "Stone_M2_Steps_MI";
		if (objName.Contains("MaterialInstanceDynamic")) continue;
		mats.Add(Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(FPaths::Combine("/Game/ValorantContent/Materials/", objName))));
	}
	return mats;
}
#undef LOCTEXT_NAMESPACE
