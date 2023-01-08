// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TBaseImporter.h"
#include "UianaSettings.h"
#include "Materials/MaterialInstanceConstant.h"
#include "AssetToolsModule.h"
#include "BPFL.h"
#include "EditorAssetLibrary.h"
#include "JsonObjectConverter.h"
#include "MaterialEditingLibrary.h"
#include "ObjectEditorUtils.h"
#include "UianaHelpers.h"
#include "Components/DecalComponent.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "HAL/FileManagerGeneric.h"
#include "Kismet2/EnumEditorUtils.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/PropertyAccessUtil.h"

/**
 * Handles importing Textures and Materials into UE with correct settings. Does not handle override materials.
 */
class MaterialImporter : public TBaseImporter<UMaterialInstanceConstant>
{
public:
	MaterialImporter();
	MaterialImporter(const UianaSettings* UianaSettings);
	void ImportMaterials();
	
	virtual bool OverrideArrayProp(const FString JsonPropName, const TSharedPtr<FJsonValue> JsonPropValue, const FProperty* ObjectProp, UMaterialInstanceConstant* BaseObj) override;
private:
	void GetTexturePaths(const TArray<FString> MatPaths, TArray<FString> &TexturePaths) const;
	void CreateMaterials(const TArray<FString> MatPaths);
	void SetMaterial(const TSharedPtr<FJsonObject> MatData, UMaterialInstanceConstant* Mat);
};
