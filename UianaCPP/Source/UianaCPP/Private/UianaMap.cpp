// Fill out your copyright notice in the Description page of Project Settings.

#include "UianaMap.h"

UUianaMap::UUianaMap(FString MapName, UUianaCPPDataSettings Settings)
{
	Name = MapName;
	// Create content directories
	UianaHelpers::CreateFolder(FolderPath, Settings.GetExportMapsFolder(), "/" + MapName);
	UianaHelpers::CreateFolder(MaterialsPath, FolderPath.Path, "/materials");
	UianaHelpers::CreateFolder(MaterialsOvrPath, FolderPath.Path, "/materials_ovr");
	UianaHelpers::CreateFolder(ObjectsPath, FolderPath.Path, "/objects");
	UianaHelpers::CreateFolder(ScenesPath, FolderPath.Path, "/scenes");
	UianaHelpers::CreateFolder(UMapsPath, FolderPath.Path, "/umaps");
	
	// Open umaps JSON file and read the UMaps to store
	FString UMapJSON;
	FFileHelper::LoadFileToString(UMapJSON, *FPaths::Combine(UMapsPath.Path, "/umaps.json"));
	TSharedPtr<FJsonObject> JsonParsed = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(UMapJSON);
	if (FJsonSerializer::Deserialize(JsonReader, JsonParsed) && JsonParsed.IsValid())
	{
		if (!JsonParsed->TryGetStringArrayField(MapName, UMaps))
		{
			UE_LOG(LogTemp, Error, TEXT("UIANA: Failed to deserialize umaps for %s"), *MapName);
		}
	}
}
