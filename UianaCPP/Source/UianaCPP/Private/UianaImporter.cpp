#include "UianaImporter.h"

FString UUianaImporter::Name = "";
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
FDirectoryPath UUianaImporter::ActorsPath = FDirectoryPath();

UUianaImporter::UUianaImporter()
{
	UE_LOG(LogTemp, Error, TEXT("Uiana: Must initialize importer with parameters!"));
}

UUianaImporter::UUianaImporter(FString MapName, UUianaCPPDataSettings Settings)
{
	Name = MapName;

	// Set paths
	FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("Uiana"))->GetContentDir();
	ToolsPath.Path = FPaths::Combine(ContentDir, "/tools"); // tools_path
	AssetsPath.Path = FPaths::Combine(ContentDir, "/assets"); // importer_assets_path
	ExportAssetsPath.Path = FPaths::Combine(Settings.ExportFolder.Path, "/export"); // assets_path
	ExportMapsPath.Path = FPaths::Combine(Settings.ExportFolder.Path, "/maps"); // maps_path
	PaksPath = Settings.PaksFolder;
	
	// Create content directories
	UianaHelpers::CreateFolder(FolderPath, ExportMapsPath.Path, "/" + MapName);
	UianaHelpers::CreateFolder(MaterialsPath, FolderPath.Path, "/materials");
	UianaHelpers::CreateFolder(MaterialsOvrPath, FolderPath.Path, "/materials_ovr");
	UianaHelpers::CreateFolder(ObjectsPath, FolderPath.Path, "/objects");
	UianaHelpers::CreateFolder(ScenesPath, FolderPath.Path, "/scenes");
	UianaHelpers::CreateFolder(UMapsPath, FolderPath.Path, "/umaps");
	UianaHelpers::CreateFolder(ActorsPath, FolderPath.Path, "/umaps");
	
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
	UBPFL::ExecuteConsoleCommand("r.DefaultFeature.LightUnits 0");
	UBPFL::ExecuteConsoleCommand("r.DynamicGlobalIlluminationMethod 0");
	TArray<FString> umapPaths;
	TArray<FString> levelPaths = {};
	TArray<FString> texturePaths = {};
	FFileManagerGeneric::Get().FindFiles(umapPaths, *(UMapsPath.Path), TEXT(".json"));
	if (NeedExport(Settings))
	{
		ExtractAssets(umapPaths);
	}
	if (!Settings.UseSubLevels)
	{
		levelPaths.Add(CreateNewLevel());
	}
	// Clear level
	UEditorActorSubsystem* actorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	actorSubsystem->DestroyActors(actorSubsystem->GetAllLevelActors());
	if (Settings.ImportMaterials)
	{
		// Import textures first
		TArray<FString> matPaths;
		IFileManager::Get().FindFiles(matPaths, *MaterialsPath.Path, true, false);
		GetTexturePaths(matPaths, texturePaths);
		matPaths.Empty();
		IFileManager::Get().FindFiles(matPaths, *MaterialsOvrPath.Path, true, false);
		GetTexturePaths(matPaths, texturePaths);
		UBPFL::ImportTextures(texturePaths);
	}
	
}

void UUianaImporter::ExtractAssets(TArray<FString> umapPaths)
{
	CUE4Extract(UMapsPath);
	UModelExtract();
	TArray<FString> actorPaths, objPaths, matPaths;
	for (FString umapPath : umapPaths)
	{
		FString umapStr;
		FFileHelper::LoadFileToString(umapStr, *umapPath);
		TArray<TSharedPtr<FJsonValue>> umapRaw, umapFiltered;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(umapStr);
		if (!FJsonSerializer::Deserialize(JsonReader, umapRaw) || !umapRaw[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize umap %s"), umapPath);
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
			FString typeLower = obj->GetStringField("Type").ToLower();
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
		FBufferArchive SaveData;
		SaveData << umapFiltered;
		FFileHelper::SaveArrayToFile(SaveData, *umapPath);
		SaveData.FlushCache();
		SaveData.Empty();

		GetObjects(actorPaths, objPaths, matPaths, umapFiltered);
	}
	// Process blueprint actors
	const TCHAR* actorPathsFilepath = *FPaths::Combine(FolderPath.Path, "/_assets_actors.txt");
	FFileHelper::SaveStringArrayToFile(actorPaths, actorPathsFilepath);
	CUE4Extract(ActorsPath, actorPathsFilepath);
	actorPaths.Empty();
	FFileManagerGeneric::Get().FindFiles(actorPaths, *(ActorsPath.Path), TEXT(".json"));
	for (FString actorPath : actorPaths)
	{
		FString actorStr;
		FFileHelper::LoadFileToString(actorStr, *actorPath);
		TArray<TSharedPtr<FJsonValue>> actorObjs;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(actorStr);
		if (!FJsonSerializer::Deserialize(JsonReader, actorObjs) || !actorObjs[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize actor %s"), actorPath);
			continue;
		}
		TArray<FString> temp;
		GetObjects(temp, objPaths, matPaths, actorObjs);
	}
	// Save asset lists
	const TCHAR* objPathsFilepath = *FPaths::Combine(FolderPath.Path, "/_assets_objects.txt");
	const TCHAR* matPathsFilepath = *FPaths::Combine(FolderPath.Path, "/_assets_materials_ovr_list.txt");
	FFileHelper::SaveStringArrayToFile(objPaths, objPathsFilepath);
	FFileHelper::SaveStringArrayToFile(objPaths, matPathsFilepath);
	CUE4Extract(ObjectsPath, objPathsFilepath);
	CUE4Extract(MaterialsOvrPath, matPathsFilepath);

	// Get models now
	TArray<FString> modelPaths;
	TArray<FString> matList = {};
	FFileManagerGeneric::Get().FindFiles(modelPaths, *(ObjectsPath.Path), TEXT(".json"));
	for (FString modelPath : modelPaths)
	{
		FString jsonStr;
		FFileHelper::LoadFileToString(jsonStr, *modelPath);
		TArray<FModelObject> modelObjects;
		FJsonObjectConverter::JsonArrayStringToUStruct<FModelObject>(jsonStr, &modelObjects, 0, 0);
		// TODO: Do I have to resave the file? L116 in liana_main.py
		// Get Object Materials
		for (FModelObject modelObject : modelObjects)
		{
			// TODO: Replace type check with ensuring only StaticMesh can pass conversion
			if (modelObject.Type == "StaticMesh")
			{
				for(FModelMaterialComponent mat : modelObject.Properties.StaticMaterials)
				{
					matList.AddUnique(FPaths::GetBaseFilename(mat.MaterialInterface.ObjectPath).Replace(TEXT("/"), TEXT("\\")));
				}
			}
		}
	}
	// Save material list
	const TCHAR* matListFilepath = *FPaths::Combine(FolderPath.Path, "/_assets_objects.txt");
	FFileHelper::SaveStringArrayToFile(matList, matListFilepath);
	CUE4Extract(MaterialsPath, matListFilepath);
	// Write exported.yo to indicate have exported
	FFileHelper::SaveStringToFile(TEXT(""), *FPaths::Combine(FolderPath.Path, "/exported.yo"));
	FFileHelper::SaveStringToFile(TEXT(""), *FPaths::Combine(ExportAssetsPath.Path, "/exported.yo"));
}

void UUianaImporter::CUE4Extract(FDirectoryPath ExportDir, FString AssetList)
{
	TArray<FStringFormatArg> args = {
		FPaths::Combine(ToolsPath.Path, "/cue4extractor.exe"),
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
	if (FPaths::FileExists(FPaths::Combine(ExportAssetsPath.Path, "/exported.yo")))
		return;
	TArray<FStringFormatArg> args = {
		FPaths::Combine(ToolsPath.Path, "/umodel.exe"),
		PaksPath.Path,
		AesKey,
		TextureFormat.Replace(TEXT("."), TEXT("")),
		ExportAssetsPath.Path
	};
	FString ConsoleCommand = FString::Format(TEXT("{0} -path={1} -game=valorant -aes={2} *.uasset -export -noanim -nooverwrite -{3} -out={4}"), args);
	GEngine->Exec(nullptr, *ConsoleCommand);
}

FString UUianaImporter::CreateNewLevel()
{
	// Get initial name
	FString initialName;
	if (Name.Contains("_"))
	{
		FString temp;
		Name.Split(TEXT("_"), &initialName, &temp);
	}
	else
	{
		initialName = Name;
	}
	TArray<FStringFormatArg> args = {initialName, Name};
	const FString levelPath = FString::Format(TEXT("/Game/ValorantContent/Maps/{0}/{1}"), args);
	UEditorAssetLibrary::LoadAsset(levelPath);
	ULevelEditorSubsystem* editorSubsystem = GEditor->GetEditorSubsystem<ULevelEditorSubsystem>();
	editorSubsystem->NewLevel(levelPath);
	return levelPath;
}

void UUianaImporter::GetTexturePaths(const TArray<FString> matPaths, TArray<FString> &texturePaths)
{
	for (FString matPath : matPaths)
	{
		FString jsonStr;
		FFileHelper::LoadFileToString(jsonStr, *matPath);
		TArray<FUianaMaterialJson> matObjects;
		FJsonObjectConverter::JsonArrayStringToUStruct<FUianaMaterialJson>(jsonStr, &matObjects, 0, 0);
		for (FUianaMaterialJson mat : matObjects)
		{
			for (FUianaTextureParameterValue param : mat.Properties.TextureParameterValues)
			{
				texturePaths.AddUnique(FPaths::Combine(ExportAssetsPath.Path, "/" +
					param.ParameterValue.ObjectPath.Replace(
					TEXT("ShooterGame\\Content"), TEXT("Game")).Replace(
						TEXT("Engine\\Content"), TEXT("Engine"))));
			}
		}
	}
}

void UUianaImporter::CreateMaterial(const TArray<FString> matPaths, TMap<FString, UMaterialInstance*> loadableMaterials)
{
	for(FString matPath : matPaths)
	{
		// TODO: Simplify CreateMaterial() and GetTexturePaths() since they share same stuff
		// Make this a function already dangnabbit
		FString jsonStr;
		FFileHelper::LoadFileToString(jsonStr, *matPath);
		TArray<FUianaMaterialJson> matObjects;
		
		FJsonObjectConverter::JsonArrayStringToUStruct<FUianaMaterialJson>(jsonStr, &matObjects, 0, 0);
		FUianaMaterialJson mat = matObjects.Pop();
		TArray<FStringFormatArg> args = {mat.Name, mat.Name};
		const FString localMatPath = FString::Format(TEXT("/Game/ValorantContent/Materials/{0}/{1}"), args);
		UMaterialInstanceConstant* matInstance = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset(localMatPath));

		FString matParent, temp;
		if (mat.Properties.Parent.ObjectName.IsEmpty())
		{
			mat.Properties.Parent.ObjectName = "Material BaseEnv_MAT_V4";
		}
		mat.Properties.Parent.ObjectName.Split(TEXT(" "), &temp, &matParent);
		if (matInstance == nullptr)
		{
			UMaterialInstanceConstantFactoryNew factory = UMaterialInstanceConstantFactoryNew();
			IAssetTools& assetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
			matInstance = static_cast<UMaterialInstanceConstant*>(assetTools.CreateAsset(mat.Name, "/Game/ValorantContent/Materials/", UMaterialInstanceConstant::StaticClass(), &factory));
		}
		UMaterialInstanceConstant* parentInstance = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/Uiana/Materials/" + matParent));
		if (parentInstance == nullptr)
		{
			parentInstance = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/Uiana/Materials/BaseEnv_MAT_V4"));
		}
		matInstance->Parent = parentInstance;
		loadableMaterials.Add(mat.Name, matInstance);
		SetMaterial(mat, matInstance);
	}
}

void UUianaImporter::SetMaterial(FUianaMaterialJson matData, UMaterialInstanceConstant* mat)
{
	SetTextures(matData, mat);
	
}

void UUianaImporter::SetTextures(FUianaMaterialJson matData, UMaterialInstanceConstant* mat)
{
	bool woodHasDiffA = false, woodNoDiffB = false, woodHasM15 = false;
	for (FUianaTextureParameterValue param : matData.Properties.TextureParameterValues)
	{
		FString textureGamePath = FPaths::ChangeExtension(param.ParameterValue.ObjectPath, TextureFormat);
		FString localPath = FPaths::Combine(ExportAssetsPath.Path, "/" + textureGamePath);
		FString paramName = param.ParameterInfo.Name.ToString().ToLower();

		if (!paramName.Contains("diffuse b low"))
		{
			if (FPaths::FileExists(localPath))
			{
				UTexture* importedTexture = static_cast<UTexture*>(
					UEditorAssetLibrary::LoadAsset(TEXT("/Game/ValorantContent/Textures/") + FPaths::GetCleanFilename(localPath)));
				if (importedTexture == nullptr) continue;
				if (paramName.Equals("diffuse") || paramName.Equals("albedo"))
				{
					UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(mat, TEXT("Diffuse"), importedTexture);
				}
				if (paramName.Equals("diffuse a") || paramName.Equals("texture a") || paramName.Equals("albedo a"))
				{
					UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(mat, TEXT("Diffuse A"), importedTexture);
				}
				if (paramName.Equals("diffuse b") || paramName.Equals("texture b") || paramName.Equals("albedo b"))
				{
					UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(mat, TEXT("Diffuse B"), importedTexture);
				}
				if (paramName.Equals("texture a normal") || paramName.Equals("normal a"))
				{
					UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(mat, TEXT("Texture A Normal"), importedTexture);
				}
				if (paramName.Equals("texture b normal") || paramName.Equals("normal b"))
				{
					UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(mat, TEXT("Texture B Normal"), importedTexture);
				}
				if (paramName.Equals("mask"))
				{
					UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(mat, TEXT("Mask Textuer"), importedTexture);
				}
				UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(mat, *paramName, importedTexture);
			}
		}

		woodHasDiffA = paramName.Contains("diffuse a") || woodHasDiffA;
		woodNoDiffB = !paramName.Contains("diffuse b") || woodNoDiffB;
		woodHasM15 = paramName.Contains("Wood_M15") || woodHasM15;
	}
	UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(mat, TEXT("WoodFix"), woodHasDiffA && woodNoDiffB && woodHasM15);
	UMaterialEditingLibrary::UpdateMaterialInstance(mat);
}

void UUianaImporter::SetMaterialSettings(FUianaMaterialProperties matProps, UMaterialInstanceConstant* mat)
{
	for (FScalarParameterValue val : matProps.ScalarParameterValues)
	{
		mat->SetScalarParameterValueEditorOnly(val.ParameterInfo, val.ParameterValue);
	}
	for (FVectorParameterValue val : matProps.VectorParameterValues)
	{
		mat->SetVectorParameterValueEditorOnly(val.ParameterInfo, val.ParameterValue);
	}
	mat->SetTextureStreamingData(matProps.TextureStreamingData);
}

bool UUianaImporter::NeedExport(UUianaCPPDataSettings const &Settings)
{
	FString exportCheckPath = FPaths::Combine(FolderPath.Path, "/exported.yo");
	bool needsExport = true;
	if(FPaths::FileExists(exportCheckPath))
	{
		FString jsonStr;
		FFileHelper::LoadFileToString(jsonStr, *exportCheckPath);
		FUianaExport exportData;
		FJsonObjectConverter::JsonObjectStringToUStruct(jsonStr, &exportData);
		needsExport = !exportData.version.Equals(Settings.ValorantVersion);
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
			if (props->HasField("StaticMesh"))
			{
				objPaths.AddUnique(FPaths::GetBaseFilename(props->GetStringField("ObjectPath")).Replace(TEXT("/"), TEXT("\\")));
				if (props->HasField("OverrideMaterials"))
				{
					for (TSharedPtr<FJsonValue, ESPMode::ThreadSafe> mat : props->GetArrayField("OverrideMaterials"))
					{
						matPaths.AddUnique(FPaths::GetBaseFilename(mat->AsObject()->GetStringField("ObjectPath")).Replace(TEXT("/"), TEXT("\\")));
					}
				}
			}
			else if (props->HasField("DecalMaterial"))
			{
				matPaths.AddUnique(FPaths::GetBaseFilename(props->GetObjectField("DecalMaterial")->GetStringField("ObjectPath")).Replace(TEXT("/"), TEXT("\\")));
			}
		}
	}
}
