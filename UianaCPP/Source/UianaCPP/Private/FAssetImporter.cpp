// Fill out your copyright notice in the Description page of Project Settings.


#include "FAssetImporter.h"

#include "UianaImporter.h"
#include "HAL/FileManagerGeneric.h"

FAssetImporter::FAssetImporter()
{
	Settings = nullptr;
}

FAssetImporter::FAssetImporter(const UianaSettings* UianaSettings)
{
	Settings = UianaSettings;
}

TArray<FString> FAssetImporter::GetExtractedUmaps()
{
	TArray<FString> UmapPaths;
	FFileManagerGeneric::Get().FindFiles(UmapPaths, *(Settings->UMapsPath.Path), TEXT(".json"));
	UianaHelpers::AddPrefixPath(Settings->UMapsPath, UmapPaths);
	if (NeedExport())
	{
		UmapPaths = ExtractAssets();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: No need to extract PAK assets, skipping."));
	}
	return UmapPaths;
}

bool FAssetImporter::NeedExport()
{
	FString ExportCheckPath = FPaths::Combine(Settings->FolderPath.Path, "exported.yo");
	bool bNeedsExport = true;
	if(FPaths::FileExists(ExportCheckPath))
	{
		FString JsonString;
		FFileHelper::LoadFileToString(JsonString, *ExportCheckPath);
		FUianaExport ExportData;
		FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &ExportData);
		bNeedsExport = !ExportData.version.Equals(Settings->ValorantVersion);
	}
	return bNeedsExport || Settings->DevForceReexport;
}

TArray<FString> FAssetImporter::ExtractAssets()
{
	TArray<FString> UmapPaths = {};
	UE_LOG(LogTemp, Warning, TEXT("Uiana: Extracting assets!"));
	Cue4Extract(Settings->UMapsPath);
	UModelExtract();
	FFileManagerGeneric::Get().FindFiles(UmapPaths, *(Settings->UMapsPath.Path), TEXT(".json"));
	UianaHelpers::AddPrefixPath(Settings->UMapsPath, UmapPaths);
	UE_LOG(LogTemp, Warning, TEXT("Uiana: Extracted %d umaps"), UmapPaths.Num());
	TArray<FString> ActorPaths, ObjPaths, MatOvrPaths;
	for (FString UmapPath : UmapPaths)
	{
		FString UmapStr;
		// FString umapPath = FPaths::Combine(UMapsPath.Path, umapName);
		FFileHelper::LoadFileToString(UmapStr, *UmapPath);
		TArray<TSharedPtr<FJsonValue>> UmapRaw, UmapFiltered;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(UmapStr);
		if (!FJsonSerializer::Deserialize(JsonReader, UmapRaw) || !UmapRaw[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize umap %s"), *UmapPath);
			continue;
		}
		
		// Filter umap
		const TArray<FString> DecalTypes = {"decalcomponent"};
		const TArray<FString> MeshTypes = {"staticmesh", "staticmeshcomponent", "instancedstaticmeshcomponent",
			"hierarchicalinstancedstaticmeshcomponent"};
		const TArray<FString> GenTypes = {"pointlightcomponent", "postprocessvolume", "culldistancevolume",
			"scenecomponent", "lightmasscharacterindirectdetailvolume", "brushcomponent", "precomputedvisibilityvolume",
			"rectlightcomponent", "spotlightcomponent", "skylightcomponent", "scenecapturecomponentcube",
			"lightmassimportancevolume", "billboardcomponent", "directionallightcomponent",
			"exponentialheightfogcomponent", "lightmassportalcomponent", "spherereflectioncapturecomponent"};
		for (TSharedPtr<FJsonValue> Component : UmapRaw)
		{
			const TSharedPtr<FJsonObject> ComponentObj = Component.Get()->AsObject();
			const FString TypeLower = ComponentObj->GetStringField("Type").ToLower();
			if (MeshTypes.Contains(TypeLower) && ComponentObj->HasField("Properties"))
			{
				UmapFiltered.Add(Component);				
			}
			else if (GenTypes.Contains(TypeLower) || DecalTypes.Contains(TypeLower))
			{
				UmapFiltered.Add(Component);
			}
			else if (TypeLower.EndsWith("_c"))
			{
				UmapFiltered.Add(Component);
			}
		}
		
		// Save cleaned-up JSON
		UianaHelpers::SaveJson(UmapFiltered, UmapPath);

		GetObjects(ActorPaths, ObjPaths, MatOvrPaths, UmapFiltered);
	}
	// Process blueprint actors
	UE_LOG(LogTemp, Display, TEXT("Uiana: Importing BP Actors!"));
	const FString ActorPathsFilepath = FPaths::Combine(Settings->FolderPath.Path, "_assets_actors.txt");
	FFileHelper::SaveStringArrayToFile(ActorPaths, *ActorPathsFilepath);
	Cue4Extract(Settings->ActorsPath, ActorPathsFilepath);
	ActorPaths.Empty();
	FFileManagerGeneric::Get().FindFiles(ActorPaths, *(Settings->ActorsPath.Path), TEXT(".json"));
	for (FString ActorPath : ActorPaths)
	{
		FString ActorStr;
		FFileHelper::LoadFileToString(ActorStr, *(Settings->ActorsPath.Path + "/" + ActorPath));
		TArray<TSharedPtr<FJsonValue>> ActorObjs;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(ActorStr);
		if (!FJsonSerializer::Deserialize(JsonReader, ActorObjs) || !ActorObjs[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize actor %s"), *ActorPath);
			continue;
		}
		TArray<FString> Temp;
		GetObjects(Temp, ObjPaths, MatOvrPaths, ActorObjs);
	}
	// Save asset lists
	const FString ObjPathsFilepath = FPaths::Combine(Settings->FolderPath.Path, "_assets_objects.txt");
	const FString MatPathsFilepath = FPaths::Combine(Settings->FolderPath.Path, "_assets_materials_ovr.txt");
	UE_LOG(LogTemp, Display, TEXT("Uiana: Saving asset file with %d assets on path: %s"), ObjPaths.Num(), *ObjPathsFilepath);
	FFileHelper::SaveStringArrayToFile(ObjPaths, *ObjPathsFilepath);
	FFileHelper::SaveStringArrayToFile(MatOvrPaths, *MatPathsFilepath);
	Cue4Extract(Settings->ObjectsPath, ObjPathsFilepath);
	Cue4Extract(Settings->MaterialsOvrPath, MatPathsFilepath);
	
	// Get models now
	UE_LOG(LogTemp, Display, TEXT("Uiana: Getting models!"));
	TArray<FString> ModelNames;
	TArray<FString> MatPaths = {};
	FFileManagerGeneric::Get().FindFiles(ModelNames, *(Settings->ObjectsPath.Path), TEXT(".json"));
	for (FString ModelName : ModelNames)
	{
		const FString ModelPath = FPaths::Combine(Settings->ObjectsPath.Path, ModelName);
		FString JsonString;
		FFileHelper::LoadFileToString(JsonString, *ModelPath);
		TArray<TSharedPtr<FJsonValue>> ModelObjs;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
		if (!FJsonSerializer::Deserialize(JsonReader, ModelObjs) || !ModelObjs[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize model %s"), *ModelPath);
			continue;
		}
		
		// Save cleaned-up JSON
		UianaHelpers::SaveJson(ModelObjs, ModelPath);
		
		// Get Object Materials
		for (TSharedPtr<FJsonValue> ModelObject : ModelObjs)
		{
			if (ModelObject.Get()->AsObject()->GetStringField("Type").Equals("StaticMesh"))
			{
				const TArray<TSharedPtr<FJsonValue>> ModelMats = ModelObject.Get()->AsObject()->GetObjectField("Properties")->GetArrayField("StaticMaterials");
				for(const TSharedPtr<FJsonValue> Mat : ModelMats)
				{
					const TSharedPtr<FJsonObject> Obj = Mat.Get()->AsObject();
					if (Obj->HasTypedField<EJson::Object>("MaterialInterface"))
					{
						const FString path = Obj->GetObjectField("MaterialInterface")->GetStringField("ObjectPath");
						MatPaths.AddUnique(FPaths::Combine(FPaths::GetPath(path), FPaths::GetBaseFilename(path)).Replace(TEXT("/"), TEXT("\\")));
					}
				}
			}
		}
	}
	// Save material list + all assets list
	TArray<FString> AllPaths = {};
	UianaHelpers::AddAllAssetPath(AllPaths, ObjPaths);
	UianaHelpers::AddAllAssetPath(AllPaths, MatPaths);
	UianaHelpers::AddAllAssetPath(AllPaths, MatOvrPaths);
	
	const FString MatListFilepath = FPaths::Combine(Settings->FolderPath.Path, "_assets_materials.txt");
	const FString AllListFilepath = FPaths::Combine(Settings->FolderPath.Path, "all_assets.txt");
	FFileHelper::SaveStringArrayToFile(MatPaths, *MatListFilepath);
	FFileHelper::SaveStringArrayToFile(AllPaths, *AllListFilepath);
	Cue4Extract(Settings->MaterialsPath, MatListFilepath);
	// Write exported.yo to indicate have exported
	FUianaExport ExportInfo;
	ExportInfo.version = Settings->ValorantVersion;
	FString ExportStr;
	FJsonObjectConverter::UStructToJsonObjectString<FUianaExport>(ExportInfo, ExportStr);
	FFileHelper::SaveStringToFile(ExportStr, *FPaths::Combine(Settings->FolderPath.Path, "exported.yo"));
	FFileHelper::SaveStringToFile(ExportStr, *FPaths::Combine(Settings->ExportAssetsPath.Path, "exported.yo"));

	return UmapPaths;
}

void FAssetImporter::GetObjects(TArray<FString> &ActorPaths, TArray<FString> &ObjPaths, TArray<FString> &MatPaths, const TArray<TSharedPtr<FJsonValue>> &JsonArr) const
{
	bool bSkippedBlueprint = false;
	for (TSharedPtr<FJsonValue> Component : JsonArr)
	{
		const TSharedPtr<FJsonObject> Obj = Component.Get()->AsObject();
		if (Obj->GetStringField("Type").EndsWith("_C") && Obj->HasField("Template"))
		{
			if (!bSkippedBlueprint)
			{
				bSkippedBlueprint = true;
				continue;
			}
			ActorPaths.AddUnique(Obj->GetStringField("Template"));
		}
		if (Obj->HasField("Properties"))
		{
			const TSharedPtr<FJsonObject> Props = Obj->GetObjectField("Properties");
			if (Props->HasTypedField<EJson::Object>("StaticMesh"))
			{
				const TSharedPtr<FJsonObject> StaticMesh = Props->GetObjectField("StaticMesh");
				if (!StaticMesh->HasField("ObjectPath")) UE_LOG(LogTemp, Error, TEXT("Uiana: No object path for static mesh %s!"), *Obj->GetStringField("Outer"));
				if (!StaticMesh->HasField("ObjectPath")) continue;
				const FString Path = StaticMesh->GetStringField("ObjectPath");
				ObjPaths.AddUnique( FPaths::Combine(FPaths::GetPath(Path), FPaths::GetBaseFilename(Path)).Replace(TEXT("/"), TEXT("\\")));
				if (Props->HasField("OverrideMaterials"))
				{
					for (TSharedPtr<FJsonValue, ESPMode::ThreadSafe> OverrideMat : Props->GetArrayField("OverrideMaterials"))
					{
						if (OverrideMat->IsNull()) continue;
						if (!OverrideMat->AsObject()->HasField("ObjectPath")) UE_LOG(LogTemp, Error, TEXT("Uiana: No object path for override materials!"));
						if (!OverrideMat->AsObject()->HasField("ObjectPath")) continue;
						const FString OverrideMatPath = OverrideMat->AsObject()->GetStringField("ObjectPath");
						MatPaths.AddUnique(FPaths::Combine(FPaths::GetPath(OverrideMatPath), FPaths::GetBaseFilename(OverrideMatPath)).Replace(TEXT("/"), TEXT("\\")));
					}
				}
			}
			else if (Props->HasField("DecalMaterial"))
			{
				if (!Props->GetObjectField("DecalMaterial")->HasField("ObjectPath")) continue;
				const FString DecalMatPath = Props->GetObjectField("DecalMaterial")->GetStringField("ObjectPath");
				MatPaths.AddUnique(FPaths::Combine(FPaths::GetPath(DecalMatPath), FPaths::GetBaseFilename(DecalMatPath)).Replace(TEXT("/"), TEXT("\\")));
			}
		}
	}
}

void FAssetImporter::Cue4Extract(const FDirectoryPath ExportDir, const FString AssetList) const
{
	TArray<FStringFormatArg> Args = {
		Settings->PaksPath.Path,
		Settings->AesKey,
		ExportDir.Path,
		Settings->Name,
		AssetList,
		Settings->UMapJsonPath.Path
	};
	const FString ConsoleCommand = FString::Format(TEXT("--game-directory \"{0}\" --aes-key {1} --export-directory \"{2}\" --map-name {3} --file-list {4} --game-umaps \"{5}\""), Args);
	UE_LOG(LogTemp, Warning, TEXT("%s %s"), *FPaths::Combine(Settings->ToolsPath.Path, "cue4extractor.exe"), *ConsoleCommand);
	FProcHandle ProcHandle = FPlatformProcess::CreateProc(*FPaths::Combine(Settings->ToolsPath.Path, "cue4extractor.exe"), *ConsoleCommand, false, false, false, nullptr, 1, nullptr, nullptr);
	if (ProcHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Ran CU4 successfully!"));
		FPlatformProcess::WaitForProc(ProcHandle);
	}
}

void FAssetImporter::Cue4Extract(const FDirectoryPath ExportDir) const
{
	TArray<FStringFormatArg> args = {
		Settings->PaksPath.Path,
		Settings->AesKey,
		ExportDir.Path,
		Settings->Name,
		Settings->UMapJsonPath.Path
	};
	const FString ConsoleCommand = FString::Format(TEXT("--game-directory \"{0}\" --aes-key {1} --export-directory \"{2}\" --map-name {3} --game-umaps \"{4}\""), args);
	UE_LOG(LogTemp, Warning, TEXT("%s %s"), *FPaths::Combine(Settings->ToolsPath.Path, "cue4extractor.exe"), *ConsoleCommand);
	FProcHandle ProcHandle = FPlatformProcess::CreateProc(*FPaths::Combine(Settings->ToolsPath.Path, "cue4extractor.exe"), *ConsoleCommand, false, false, false, nullptr, 1, nullptr, nullptr);
	if (ProcHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Ran CU4 successfully!"));
		FPlatformProcess::WaitForProc(ProcHandle);
	}
}

void FAssetImporter::UModelExtract() const
{
	const FString AllAssetPath = FPaths::Combine(Settings->FolderPath.Path, "all_assets.txt");
	TArray<FStringFormatArg> Args = {
		Settings->PaksPath.Path,
		Settings->AesKey,
		AllAssetPath,
		Settings->TextureFormat.Replace(TEXT("."), TEXT("")),
		Settings->ExportAssetsPath.Path
	};
	const FString ConsoleCommand = FString::Format(TEXT("-path=\"{0}\" -game=valorant -aes={1} -files=\"{2}\" -export -{3} -out=\"{4}\" *"), Args);
	UE_LOG(LogTemp, Warning, TEXT("%s %s"), *FPaths::Combine(Settings->ToolsPath.Path, "umodel.exe"), *ConsoleCommand);
	FProcHandle ProcHandle = FPlatformProcess::CreateProc(*FPaths::Combine(Settings->ToolsPath.Path, "umodel.exe"), *ConsoleCommand, false, false, false, nullptr, 1, nullptr, nullptr);
	if (ProcHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Ran UModel successfully!"));
		FPlatformProcess::WaitForProc(ProcHandle);
	}
}
