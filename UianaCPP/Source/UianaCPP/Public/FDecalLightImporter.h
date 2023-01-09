#pragma once
#include "TBaseImporter.h"
#include "EditorAssetLibrary.h"
#include "EditorClassUtils.h"
#include "EditorLevelLibrary.h"
#include "UianaHelpers.h"
#include "Components/BrushComponent.h"
#include "Engine/DecalActor.h"
#include "Engine/TextureCube.h"
#include "Materials/MaterialInstanceConstant.h"
#if ENGINE_MAJOR_VERSION == 5
#include "Subsystems/EditorActorSubsystem.h"
#endif

class FDecalLightImporter final : TBaseImporter<UActorComponent>
{
public:
	FDecalLightImporter();
	FDecalLightImporter(const UianaSettings* UianaSettings);
	void ImportDecal(const TSharedPtr<FJsonObject> Obj);
	void ImportLight(const TSharedPtr<FJsonObject> Obj);
protected:
	virtual bool OverrideNumericProp(const FString JsonPropName, const TSharedPtr<FJsonValue> JsonPropValue, const FProperty* ObjectProp, UActorComponent* BaseObj) override;
	virtual bool OverrideObjectProp(const FString JsonPropName, const TSharedPtr<FJsonValue> JsonPropValue, const FProperty* ObjectProp, UActorComponent* BaseObj) override;
};
