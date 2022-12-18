// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetImporter.h"

#include "UianaImporter.h"
#include "HAL/FileManagerGeneric.h"

AssetImporter::AssetImporter()
{
	Settings = nullptr;
}

AssetImporter::AssetImporter(const UUianaSettings* UianaSettings)
{
	Settings = UianaSettings;
}

TArray<FString> AssetImporter::GetExtractedUmaps()
{
	TArray<FString> umapPaths;
	FFileManagerGeneric::Get().FindFiles(umapPaths, *(Settings->UMapsPath.Path), TEXT(".json"));
	UianaHelpers::AddPrefixPath(Settings->UMapsPath, umapPaths);
	if (NeedExport())
	{
		umapPaths = ExtractAssets();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: No need to extract PAK assets, skipping."));
	}
	return umapPaths;
}

bool AssetImporter::NeedExport()
{
	FString exportCheckPath = FPaths::Combine(Settings->FolderPath.Path, "exported.yo");
	bool needsExport = true;
	if(FPaths::FileExists(exportCheckPath))
	{
		FString jsonStr;
		FFileHelper::LoadFileToString(jsonStr, *exportCheckPath);
		FUianaExport exportData;
		FJsonObjectConverter::JsonObjectStringToUStruct(jsonStr, &exportData);
		needsExport = !exportData.version.Equals(Settings->ValorantVersion);
	}
	return needsExport || Settings->DevForceReexport;
}

TArray<FString> AssetImporter::ExtractAssets()
{
	TArray<FString> umapPaths = {};
	UE_LOG(LogTemp, Warning, TEXT("Uiana: Extracting assets!"));
	CUE4Extract(Settings->UMapsPath);
	UModelExtract();
	FFileManagerGeneric::Get().FindFiles(umapPaths, *(Settings->UMapsPath.Path), TEXT(".json"));
	UianaHelpers::AddPrefixPath(Settings->UMapsPath, umapPaths);
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
	const FString actorPathsFilepath = FPaths::Combine(Settings->FolderPath.Path, "_assets_actors.txt");
	FFileHelper::SaveStringArrayToFile(actorPaths, *actorPathsFilepath);
	CUE4Extract(Settings->ActorsPath, actorPathsFilepath);
	actorPaths.Empty();
	FFileManagerGeneric::Get().FindFiles(actorPaths, *(Settings->ActorsPath.Path), TEXT(".json"));
	for (FString actorPath : actorPaths)
	{
		FString actorStr;
		FFileHelper::LoadFileToString(actorStr, *(Settings->ActorsPath.Path + "/" + actorPath));
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
	const FString objPathsFilepath = FPaths::Combine(Settings->FolderPath.Path, "_assets_objects.txt");
	const FString matPathsFilepath = FPaths::Combine(Settings->FolderPath.Path, "_assets_materials_ovr.txt");
	UE_LOG(LogTemp, Display, TEXT("Uiana: Saving asset file with %d assets on path: %s"), objPaths.Num(), *objPathsFilepath);
	FFileHelper::SaveStringArrayToFile(objPaths, *objPathsFilepath);
	FFileHelper::SaveStringArrayToFile(matOvrPaths, *matPathsFilepath);
	CUE4Extract(Settings->ObjectsPath, objPathsFilepath);
	CUE4Extract(Settings->MaterialsOvrPath, matPathsFilepath);
	
	// Get models now
	UE_LOG(LogTemp, Display, TEXT("Uiana: Getting models!"));
	TArray<FString> modelNames;
	TArray<FString> matPaths = {};
	FFileManagerGeneric::Get().FindFiles(modelNames, *(Settings->ObjectsPath.Path), TEXT(".json"));
	for (FString modelName : modelNames)
	{
		const FString modelPath = FPaths::Combine(Settings->ObjectsPath.Path, modelName);
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
	
	const FString matListFilepath = FPaths::Combine(Settings->FolderPath.Path, "_assets_materials.txt");
	const FString allListFilepath = FPaths::Combine(Settings->FolderPath.Path, "all_assets.txt");
	FFileHelper::SaveStringArrayToFile(matPaths, *matListFilepath);
	FFileHelper::SaveStringArrayToFile(allPaths, *allListFilepath);
	CUE4Extract(Settings->MaterialsPath, matListFilepath);
	// Write exported.yo to indicate have exported
	FUianaExport exportInfo;
	exportInfo.version = Settings->ValorantVersion;
	FString exportStr;
	FJsonObjectConverter::UStructToJsonObjectString<FUianaExport>(exportInfo, exportStr);
	FFileHelper::SaveStringToFile(exportStr, *FPaths::Combine(Settings->FolderPath.Path, "exported.yo"));
	FFileHelper::SaveStringToFile(exportStr, *FPaths::Combine(Settings->ExportAssetsPath.Path, "exported.yo"));

	return umapPaths;
}

void AssetImporter::GetObjects(TArray<FString> &actorPaths, TArray<FString> &objPaths, TArray<FString> &matPaths, const TArray<TSharedPtr<FJsonValue>> &jsonArr)
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

void AssetImporter::CUE4Extract(const FDirectoryPath ExportDir, const FString AssetList)
{
	TArray<FStringFormatArg> args = {
		Settings->PaksPath.Path,
		Settings->AesKey,
		ExportDir.Path,
		Settings->Name,
		AssetList,
		Settings->UMapJsonPath.Path
	};
	FString ConsoleCommand = FString::Format(TEXT("--game-directory \"{0}\" --aes-key {1} --export-directory \"{2}\" --map-name {3} --file-list {4} --game-umaps \"{5}\""), args);
	UE_LOG(LogTemp, Warning, TEXT("%s %s"), *FPaths::Combine(Settings->ToolsPath.Path, "cue4extractor.exe"), *ConsoleCommand);
	FProcHandle handle = FPlatformProcess::CreateProc(*FPaths::Combine(Settings->ToolsPath.Path, "cue4extractor.exe"), *ConsoleCommand, false, false, false, nullptr, 1, nullptr, nullptr);
	if (handle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Ran CU4 successfully!"));
		FPlatformProcess::WaitForProc(handle);
	}
}

void AssetImporter::CUE4Extract(const FDirectoryPath ExportDir)
{
	TArray<FStringFormatArg> args = {
		Settings->PaksPath.Path,
		Settings->AesKey,
		ExportDir.Path,
		Settings->Name,
		Settings->UMapJsonPath.Path
	};
	FString ConsoleCommand = FString::Format(TEXT("--game-directory \"{0}\" --aes-key {1} --export-directory \"{2}\" --map-name {3} --game-umaps \"{4}\""), args);
	UE_LOG(LogTemp, Warning, TEXT("%s %s"), *FPaths::Combine(Settings->ToolsPath.Path, "cue4extractor.exe"), *ConsoleCommand);
	FProcHandle handle = FPlatformProcess::CreateProc(*FPaths::Combine(Settings->ToolsPath.Path, "cue4extractor.exe"), *ConsoleCommand, false, false, false, nullptr, 1, nullptr, nullptr);
	if (handle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Ran CU4 successfully!"));
		FPlatformProcess::WaitForProc(handle);
	}
}

void AssetImporter::UModelExtract()
{
	const FString allAssetPath = FPaths::Combine(Settings->FolderPath.Path, "all_assets.txt");
	TArray<FStringFormatArg> args = {
		Settings->PaksPath.Path,
		Settings->AesKey,
		allAssetPath,
		Settings->TextureFormat.Replace(TEXT("."), TEXT("")),
		Settings->ExportAssetsPath.Path
	};
	const FString ConsoleCommand = FString::Format(TEXT("-path=\"{0}\" -game=valorant -aes={1} -files=\"{2}\" -export -{3} -out=\"{4}\" *"), args);
	UE_LOG(LogTemp, Warning, TEXT("%s %s"), *FPaths::Combine(Settings->ToolsPath.Path, "umodel.exe"), *ConsoleCommand);
	FProcHandle handle = FPlatformProcess::CreateProc(*FPaths::Combine(Settings->ToolsPath.Path, "umodel.exe"), *ConsoleCommand, false, false, false, nullptr, 1, nullptr, nullptr);
	if (handle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Ran UModel successfully!"));
		FPlatformProcess::WaitForProc(handle);
	}
}