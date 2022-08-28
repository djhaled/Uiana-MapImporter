// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UianaCPPDataSettings.h"
#include "UObject/Object.h"
#include "Misc/Paths.h"
#include "JsonUtilities.h"
#include "UianaMap.generated.h"

/**
 * Represents a Map imported by Uiana. Stores map-specific settings.
 */
UCLASS()
class UIANACPP_API UUianaMap : public UObject
{
	GENERATED_BODY()
public:
	UUianaMap(FString MapName, UUianaCPPDataSettings Settings);
	FString Name;
	TArray<FString> UMaps;
	FDirectoryPath FolderPath, MaterialsPath, MaterialsOvrPath, ObjectsPath, ScenesPath, UMapsPath;
};
