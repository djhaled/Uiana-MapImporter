#include "FDecalLightImporter.h"

FDecalLightImporter::FDecalLightImporter() : TBaseImporter<UActorComponent>()
{
}

FDecalLightImporter::FDecalLightImporter(const UianaSettings* UianaSettings) : TBaseImporter<UActorComponent>(UianaSettings)
{
}

void FDecalLightImporter::ImportDecal(const TSharedPtr<FJsonObject> Obj)
{
	if (Obj->HasField("Template") || !UianaHelpers::HasTransformComponent(Obj->GetObjectField("Properties"))) return;
	ADecalActor* decalActor = Cast<ADecalActor>(UEditorLevelLibrary::SpawnActorFromClass(ADecalActor::StaticClass(), FVector::ZeroVector));
	decalActor->SetFolderPath("Decals");
	decalActor->SetActorLabel(Obj->GetStringField("Name"));
	FString decalName = FPaths::GetBaseFilename(Obj->GetObjectField("Properties")->GetObjectField("DecalMaterial")->GetStringField("ObjectPath"));
	UMaterialInstanceConstant* decalMat = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Materials/" + decalName + "." + decalName));
	decalActor->SetDecalMaterial(decalMat);
	SetSettingsFromJsonProperties(Obj->GetObjectField("Properties"), Cast<UActorComponent>(decalActor->GetDecal()));
}

void FDecalLightImporter::ImportLight(const TSharedPtr<FJsonObject> Obj)
{
	const FString lightType = Obj->GetStringField("Type").Replace(TEXT("Component"), TEXT(""), ESearchCase::CaseSensitive);
	UClass* lightClass = FEditorClassUtils::GetClassFromString(lightType);
	UEditorActorSubsystem* EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	AActor* light = EditorActorSubsystem->SpawnActorFromClass(lightClass, FVector::ZeroVector);
	light->SetFolderPath(FName("Lights/" + lightType));
	light->SetActorLabel(Obj->GetStringField("Name"));
	UActorComponent* lightComponent = light->GetRootComponent();
	if (Cast<UBrushComponent>(lightComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Casting BrushComponent's parent as UObject!"));
		lightComponent = Cast<UActorComponent>(light);
		if (lightComponent == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to convert parent of BrushComponent!"));
			return;
		}
		const FProperty* settingsProp = PropertyAccessUtil::FindPropertyByName("Settings", lightComponent->GetClass());
		if (settingsProp != nullptr)
		{
			UianaHelpers::SetActorProperty(lightComponent->GetClass(), lightComponent, "Unbound", true);
			UianaHelpers::SetActorProperty(lightComponent->GetClass(), lightComponent, "Priority", 1.0);
			SetSettingsFromJsonProperties(Obj->GetObjectField("Properties")->GetObjectField("Settings"), lightComponent);
		}
		SetSettingsFromJsonProperties(Obj->GetObjectField("Properties"), lightComponent);
		return;
	}
	const FProperty* settingsProp = PropertyAccessUtil::FindPropertyByName("Settings", lightComponent->GetClass());
	if (settingsProp != nullptr)
	{
		UianaHelpers::SetActorProperty(lightComponent->GetClass(), lightComponent, "Unbound", true);
		UianaHelpers::SetActorProperty(lightComponent->GetClass(), lightComponent, "Priority", 1.0);
		SetSettingsFromJsonProperties(Obj->GetObjectField("Properties")->GetObjectField("Settings"), lightComponent);
	}
	SetSettingsFromJsonProperties(Obj->GetObjectField("Properties"), lightComponent);
}

bool FDecalLightImporter::OverrideNumericProp(const FString JsonPropName, const TSharedPtr<FJsonValue> JsonPropValue,
                                                 const FProperty* ObjectProp, UActorComponent* BaseObj)
{
	if (const FFloatProperty* FloatProperty = CastField<FFloatProperty>(ObjectProp))
	{
		if (JsonPropName.Equals("InfluenceRadius") && JsonPropValue.Get()->AsNumber() == 0)
		{
			// Not Meshes, Decals | Lights
			FloatProperty->SetPropertyValue_InContainer(BaseObj, 14680.0);
			return true;
		}
	}
	return false;
}

bool FDecalLightImporter::OverrideObjectProp(const FString JsonPropName, const TSharedPtr<FJsonValue> JsonPropValue,
	const FProperty* ObjectProp, UActorComponent* BaseObj)
{
	if (JsonPropName.Equals("IESTexture"))
	{
		// Not Meshes, Decals | Lights
		FString temp, newTextureName;
		JsonPropValue.Get()->AsObject()->GetStringField("ObjectName").Split("_", &temp, &newTextureName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		FString assetPath = "/UianaCPP/IESProfiles/" + newTextureName + "." + newTextureName;
		UTexture* newTexture = Cast<UTexture>(UEditorAssetLibrary::LoadAsset(assetPath));
		UianaHelpers::SetActorProperty<UTexture*>(BaseObj->GetClass(), BaseObj, JsonPropName, newTexture);
	}
	else if (JsonPropName.Equals("Cubemap"))
	{
		// Not Meshes, Decals | Lights
		FString newCubemapName = JsonPropValue.Get()->AsObject()->GetStringField("ObjectName").Replace(TEXT("TextureCube "), TEXT(""));
		FString assetPath = "/UianaCPP/CubeMaps/" + newCubemapName + "." + newCubemapName;
		// TODO: Convert all static_cast with UObjects to Cast<>()
		UTextureCube* newCube = Cast<UTextureCube>(UEditorAssetLibrary::LoadAsset(assetPath));
		UianaHelpers::SetActorProperty<UTextureCube*>(BaseObj->GetClass(), BaseObj, JsonPropName, newCube);
	}
	// else if (JsonPropName.Equals("DecalMaterial")) // Used by Decals, cannot easily get working but seems fine anyway
	// {
	// 	// TODO: Implement remaining object imports. Line 303 in main.py
	// 	FString decalPath = FPaths::GetPath(JsonPropValue.Get()->AsObject()->GetStringField("ObjectPath"));
	// 	UMaterialInstanceConstant* decalMat = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Materials/" + decalPath + "." + decalPath));
	// 	UDecalComponent decalComponent;
	// 	decalComponent.SetMaterial(0, bp);
	// 	decalComponent.SetDecalMaterial(decalMat);
	// }
	else
	{
		return false;
	}
	return true;
}