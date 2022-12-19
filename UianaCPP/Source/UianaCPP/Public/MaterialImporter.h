// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UianaSettings.h"
#include "Materials/MaterialInstanceConstant.h"

/**
 * 
 */
class MaterialImporter
{
public:
	MaterialImporter();
	MaterialImporter(const UianaSettings* UianaSettings);
	void ImportMaterials();

	static TArray<UMaterialInterface*> CreateOverrideMaterials(const TSharedPtr<FJsonObject> obj);
private:
	const UianaSettings* Settings;
	void GetTexturePaths(const TArray<FString> matPaths, TArray<FString> &texturePaths);
	void CreateMaterials(const TArray<FString> matPaths);
	void SetMaterial(const TSharedPtr<FJsonObject> matData, UMaterialInstanceConstant* mat);
	void SetTextures(const TSharedPtr<FJsonObject> matData, UMaterialInstanceConstant* mat);
	void SetMaterialSettings(const TSharedPtr<FJsonObject> matProps, UMaterialInstanceConstant* mat);
	FMaterialInstanceBasePropertyOverrides SetBasePropertyOverrides(const TSharedPtr<FJsonObject> matProps);
};
