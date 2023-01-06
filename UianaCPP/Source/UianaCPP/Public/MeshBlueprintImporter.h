// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TBaseImporter.h"
#include "UianaSettings.h"
#include "Engine/SCS_Node.h"
#include "AssetToolsModule.h"
#include "BPFL.h"
#include "EditorAssetLibrary.h"
#include "EditorClassUtils.h"
#include "EditorLevelLibrary.h"
#include "HismActorCPP.h"
#include "JsonObjectConverter.h"
#include "MaterialImporter.h"
#include "UianaHelpers.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Factories/BlueprintFactory.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/PropertyAccessUtil.h"
/**
 * 
 */
class MeshBlueprintImporter : TBaseImporter<UActorComponent>
{
public:
	MeshBlueprintImporter();
	explicit MeshBlueprintImporter(const UianaSettings* UianaSettings);
	void CreateBlueprints(const TArray<FString> BPPaths);
	void ImportBlueprint(const TSharedPtr<FJsonObject> Obj, TMap<FString, AActor*> &BPMapping);
	void ImportMesh(const TSharedPtr<FJsonObject> Obj, const FString UmapName, const TMap<FString, AActor*> BPMapping);
protected:
	virtual bool OverrideObjectProp(const FString JsonPropName, const TSharedPtr<FJsonValue> JsonPropValue, const FProperty* ObjectProp, UActorComponent* BaseObj) override;
	virtual bool OverrideArrayProp(const FString JsonPropName, const TSharedPtr<FJsonValue> JsonPropValue, const FProperty* ObjectProp, UActorComponent* BaseObj) override;
private:
	TArray<USCS_Node*> GetLocalBPChildren(TArray<TSharedPtr<FJsonValue>> ChildNodes, TArray<TSharedPtr<FJsonValue>> BPData, UBlueprint* BPActor);
	void FixActorBP(const TSharedPtr<FJsonObject> BPData, const TMap<FString, AActor*> BPMapping, bool bImportMaterials);
	TArray<UMaterialInterface*> CreateOverrideMaterials(const TSharedPtr<FJsonObject> Obj);
};
