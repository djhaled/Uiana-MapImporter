// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UianaSettings.h"
#include "Engine/SCS_Node.h"

/**
 * 
 */
class BlueprintImporter
{
public:
	BlueprintImporter();
	BlueprintImporter(const UUianaSettings* UianaSettings);
	void CreateBlueprints(const UUianaSettings* Settings, const TArray<FString> bpPaths);
	void ImportBlueprint(const TSharedPtr<FJsonObject> obj, TMap<FString, AActor*> &bpMapping);
	static void FixActorBP(const TSharedPtr<FJsonObject> bpData, const TMap<FString, AActor*> bpMapping, bool bImportMaterials);
private:
	const UUianaSettings* Settings;
	TArray<USCS_Node*> GetLocalBPChildren(TArray<TSharedPtr<FJsonValue>> childNodes, TArray<TSharedPtr<FJsonValue>> bpData, UBlueprint* bpActor);
	void SetBPSettings(const TSharedPtr<FJsonObject> bpProps, UActorComponent* bp);
};
