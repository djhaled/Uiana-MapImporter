// Fill out your copyright notice in the Description page of Project Settings.


#include "UianaMap.h"

UUianaMap::UUianaMap(FString MapName, UUianaCPPDataSettings Settings)
{
	// TODO: Create Folders. Python code: helpers:531
	this->Name = MapName;
	this->FolderPath.Path = FPaths::Combine(Settings.ExportFolder, "/maps/", MapName);
	this->MaterialsPath.Path = FPaths::Combine(this->FolderPath.Path, "/materials");
	this->MaterialsOvrPath.Path = FPaths::Combine(this->FolderPath.Path, "/materials_ovr");
	this->ObjectsPath.Path = FPaths::Combine(this->FolderPath.Path, "/objects");
	this->ScenesPath.Path = FPaths::Combine(this->FolderPath.Path, "/scenes");
	this->UMapsPath.Path = FPaths::Combine(this->FolderPath.Path, "/umaps");

	// Open umaps JSON file and read the UMaps to store
	FString UMapJSON;
	FFileHelper::LoadFileToString(UMapJSON, *FPaths::Combine(this->UMapsPath.Path, "/umaps.json"));
	TSharedPtr<FJsonObject> JsonParsed = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(UMapJSON);
	if (FJsonSerializer::Deserialize(JsonReader, JsonParsed) && JsonParsed.IsValid())
	{
		if (!JsonParsed->TryGetStringArrayField(MapName, this->UMaps))
		{
			UE_LOG(LogTemp, Error, TEXT("UIANA: Failed to deserialize umaps for %s"), *MapName);
		}
	}
}
