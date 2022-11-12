#include "UianaImporter.h"

#include "IHeadMountedDisplay.h"
#include "Kismet2/StructureEditorUtils.h"

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
	// Need to set to lowercase to be used for comparisons
	for (int i = 0; i < BlacklistedObjs.Num(); i++) BlacklistedObjs[i].ToLowerInline();
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
		CreateMaterial(matPaths);
		CreateMaterial(matOvrPaths);
	}
	// // TODO: Implement import mesh
	// if (Settings->ImportMeshes)
	// {
	// 	
	// }
	// // TODO: Implement import BP
	// FScopedSlowTask UianaTask(umapPaths.Num(), LOCTEXT ("UianaTask", "Importing Map"));
	// UianaTask.MakeDialog();
	// for (FString umap : umapPaths)
	// {
	// 	for (int i = umapPaths.Num() - 1; i >= 0; i--)
	// 	{
	// 		FString umapStr;
	// 		FFileHelper::LoadFileToString(umapStr, *umapPaths[i]);
	// 		TArray<TSharedPtr<FJsonValue>> umapData, umapFiltered;
	// 		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(umapStr);
	// 		if (!FJsonSerializer::Deserialize(JsonReader, umapData) || !umapData[0].IsValid())
	// 		{
	// 			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize umap %s"), *umapPaths[i]);
	// 			continue;
	// 		}
	// 		FString umapName = FPaths::GetBaseFilename(umapPaths[i]);
	// 		// TODO: Add level name to format text
	// 		UianaTask.EnterProgressFrame(1, FText::Format(LOCTEXT("UianaTask", "Importing level {1}/{2}"), umapPaths.Num() - i, umapPaths.Num()));
	// 		ImportUmap(umapData, umapName);
	// 		// TODO: Add import sublevel logic line 799 main.py
	// 	}
	// 	// TODO: Add import sublevel logic line 803 main.py
	// 	// TODO: Add import mesh logic line 805 main.py
	// }
}

void UUianaImporter::ExtractAssets(TArray<FString> umapNames)
{
	UE_LOG(LogTemp, Warning, TEXT("Uiana: Extracting assets!"));
	CUE4Extract(UMapsPath);
	UModelExtract();
	UE_LOG(LogTemp, Warning, TEXT("Uiana: Extracted %d umaps"), umapNames.Num());
	TArray<FString> actorPaths, objPaths, matOvrPaths;
	for (FString umapName : umapNames)
	{
		FString umapStr;
		FString umapPath = FPaths::Combine(UMapsPath.Path, umapName);
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
					UE_LOG(LogTemp, Display, TEXT("Uiana: Adding texture path %s"), *texturePath);
					texturePaths.AddUnique(texturePath);
				}
			}
		}
	}
}

void UUianaImporter::CreateMaterial(const TArray<FString> matPaths)
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
	// TODO: #1 Get base property overrides working
	if (matProps.Get()->HasTypedField<EJson::Object>("BasePropertyOverrides"))
	{
		FMaterialInstanceBasePropertyOverrides overrides = SetBasePropertyOverrides(matProps.Get()->GetObjectField("BasePropertyOverrides"));
		mat->BasePropertyOverrides = overrides;
		UMaterialEditingLibrary::UpdateMaterialInstance(mat);
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
			UMaterialEditingLibrary::UpdateMaterialInstance(mat);
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
			UMaterialEditingLibrary::UpdateMaterialInstance(mat);
		}
	}
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
			UE_LOG(LogTemp, Display, TEXT("Uiana: Setting texture %s"), *texturePath);
			UTexture* loadedTexture = static_cast<UTexture*>(
					UEditorAssetLibrary::LoadAsset(texturePath));
			if (loadedTexture == nullptr) continue;
			UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(mat, *paramName, loadedTexture);
		}
		else UE_LOG(LogTemp, Warning, TEXT("Uiana: Missing texture at expected directory %s"), *FPaths::GetPath(localPath));
	}
	UMaterialEditingLibrary::UpdateMaterialInstance(mat);
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
		else if (propType == EJson::Object)
		{
			if (prop.Key.Equals("IESTexture"))
			{
				FString temp, newTextureName;
				propValue.Get()->AsObject()->GetStringField("ObjectName").Split("_", &temp, &newTextureName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
				FString assetPath = "/UianaCPP/IESProfiles/" + newTextureName + "." + newTextureName;
				UTexture* newTexture = static_cast<UTexture*>(UEditorAssetLibrary::LoadAsset(assetPath));
				UE_LOG(LogTemp, Display, TEXT("Uiana: Overrides struct includes %s!"), *prop.Key);
				// FObjectEditorUtils::SetPropertyValue(mat, propName, newTexture);
			}
			else if (prop.Key.Equals("Cubemap"))
			{
				FString newCubemapName = propValue.Get()->AsObject()->GetStringField("ObjectName").Replace(TEXT("TextureCube "), TEXT(""));
				FString assetPath = "/UianaCPP/CubeMaps/" + newCubemapName + "." + newCubemapName;
				// TODO: Convert all static_cast with UObjects to Cast<>()
				UTextureCube* newCube = Cast<UTextureCube, UObject>(UEditorAssetLibrary::LoadAsset(assetPath));
				// FObjectEditorUtils::SetPropertyValue(mat, propName, newCube);
				UE_LOG(LogTemp, Display, TEXT("Uiana: Overrides struct includes %s!"), *prop.Key);
			}
			// else if (prop.Key.Equals("DecalMaterial"))
			// {
			// 	FString decalPath = FPaths::GetPath(propValue.Get()->AsObject()->GetStringField("ObjectPath"));
			// 	UMaterialInstanceConstant* decalMat = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Materials/" + decalPath + "." + decalPath));
			// 	UDecalComponent decalComponent;
			// 	decalComponent.SetMaterial(0, mat);
			// 	decalComponent.SetDecalMaterial(decalMat);
			// }
			else if (prop.Key.Equals("DecalSize"))
			{
				const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
				const TArray<FName> params = {"X", "Y", "Z"};
				// UianaHelpers::SetStructPropertiesFromJson<FDoubleProperty, double>(&overrides, objectProp, obj, params);
				UE_LOG(LogTemp, Display, TEXT("Uiana: Overrides struct includes %s!"), *prop.Key);
			}
			else if (prop.Key.Equals("StaticMesh"))
			{
				FString meshName;
				if (propValue->AsObject()->TryGetStringField("ObjectName", meshName))
				{
					FString name = meshName.Replace(TEXT("StaticMesh "), TEXT(""), ESearchCase::CaseSensitive);
					UStaticMesh* mesh = static_cast<UStaticMesh*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Meshes/" + name));
					// FObjectEditorUtils::SetPropertyValue(mat, propName, mesh);
					UE_LOG(LogTemp, Display, TEXT("Uiana: Overrides struct includes %s!"), *prop.Key);
				}
			}
			else if (prop.Key.Equals("BoxExtent"))
			{
				const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
				const TArray<FName> params = {"X", "Y", "Z"};
				// UianaHelpers::SetStructPropertiesFromJson<FDoubleProperty, double>(&overrides, objectProp, obj, params);
				UE_LOG(LogTemp, Display, TEXT("Uiana: Overrides struct includes %s!"), *prop.Key);
			}
			else if (prop.Key.Equals("LightmassSettings"))
			{
				UE_LOG(LogTemp, Display, TEXT("Uiana: Overrides struct includes %s!"), *prop.Key);
				// TODO: Create lightmass settings
				
			}
		}
		else if (propType == EJson::Array && prop.Key.Equals("OverrideMaterials"))
		{
			TArray<UMaterialInstanceConstant*> overrideMats;
			UMaterialInstanceConstant* loaded = nullptr;
			for (TSharedPtr<FJsonValue, ESPMode::ThreadSafe> ovrMat : propValue.Get()->AsArray())
			{
				const TSharedPtr<FJsonObject> matData = ovrMat.Get()->AsObject();
				FString temp, objName;
				matData.Get()->GetStringField("ObjectName").Split(" ", &temp, &objName, ESearchCase::Type::IgnoreCase, ESearchDir::FromEnd);
				if (objName.Contains("MaterialInstanceDynamic")) continue;
				loaded = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/UianaCPP/Materials/" + objName));
				if (loaded == nullptr) loaded = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantMaterials/Materials/" + objName));
				overrideMats.Add(loaded);
			}
			UE_LOG(LogTemp, Display, TEXT("Uiana: Overrides struct includes %s!"), *prop.Key);
			// FObjectEditorUtils::SetPropertyValue(mat, propName, overrideMats);
		}
		else
		{
			// TODO: Set Unreal Enum somehow line 351
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
				FObjectEditorUtils::SetPropertyValue(mat, propName, vec);
			}
			else if (prop.Key.Equals("LightmassSettings"))
			{
				// TODO: Create lightmass settings
				
			}
		}
		else if (propType == EJson::Array && prop.Key.Equals("OverrideMaterials"))
		{
			// TODO: Create ovr material 325
			TArray<UMaterialInstanceConstant*> overrideMats;
			UMaterialInstanceConstant* loaded = nullptr;
			for (TSharedPtr<FJsonValue, ESPMode::ThreadSafe> ovrMat : propValue.Get()->AsArray())
			{
				const TSharedPtr<FJsonObject> matData = ovrMat.Get()->AsObject();
				FString temp, objName;
				matData.Get()->GetStringField("ObjectName").Split(" ", &temp, &objName, ESearchCase::Type::IgnoreCase, ESearchDir::FromEnd);
				if (objName.Contains("MaterialInstanceDynamic")) continue;
				loaded = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/UianaCPP/Materials/" + objName));
				if (loaded == nullptr) loaded = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantMaterials/Materials/" + objName));
				overrideMats.Add(loaded);
			}
			FObjectEditorUtils::SetPropertyValue(mat, propName, overrideMats);
		}
		else
		{
			// TODO: Set Unreal Enum somehow line 351
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
				if (!staticMesh->HasField("ObjectPath")) UE_LOG(LogTemp, Display, TEXT("Uiana: No object path for static mesh %s!"), *obj->GetStringField("Outer"));
				if (!staticMesh->HasField("ObjectPath")) continue;
				const FString path = staticMesh->GetStringField("ObjectPath");
				objPaths.AddUnique( FPaths::Combine(FPaths::GetPath(path), FPaths::GetBaseFilename(path)).Replace(TEXT("/"), TEXT("\\")));
				if (props->HasField("OverrideMaterials"))
				{
					for (TSharedPtr<FJsonValue, ESPMode::ThreadSafe> mat : props->GetArrayField("OverrideMaterials"))
					{
						if (mat->IsNull()) continue;
						if (!mat->AsObject()->HasField("ObjectPath")) UE_LOG(LogTemp, Display, TEXT("Uiana: No object path for override materials!"));
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
//
// void UUianaImporter::ImportUmap(const TArray<TSharedPtr<FJsonValue>> umapData, const FString umapName)
// {
// 	// Filter objects
// 	for (const TSharedPtr<FJsonValue> objData : umapData)
// 	{
// 		const TSharedPtr<FJsonObject> obj = objData.Get()->AsObject();
// 		FString objName;
// 		if (!obj.Get()->HasField("Properties")) objName = "None";
// 		else if (obj.Get()->GetObjectField("Properties")->HasField("StaticMesh"))
// 		{
// 			objName = obj.Get()->GetObjectField("Properties")->GetObjectField("StaticMesh")->GetStringField("ObjectPath");
// 		}
// 		else if (obj.Get()->HasField("Outer"))
// 		{
// 			objName = obj.Get()->GetStringField("Outer");
// 		}
// 		else continue;
// 		objName = FPaths::GetCleanFilename(objName).ToLower();
// 		if (BlacklistedObjs.Contains(objName)) continue;
// 		// TODO: Add importing blueprint logic here line 504 main.py
// 		// TODO: Add importing mesh logic here line 512 main.py
// 		// TODO: Add importing decal logic here line 515 main.py
// 		// TODO: Add importing light logic here line 517 main.py
// 	}
// 	
// }
//
//
// #undef LOCTEXT_NAMESPACE