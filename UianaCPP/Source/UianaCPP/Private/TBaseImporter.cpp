#include "TBaseImporter.h"

template<class ObjType>
TBaseImporter<ObjType>::TBaseImporter()
{
	Settings = nullptr;
}

template<class ObjType>
TBaseImporter<ObjType>::TBaseImporter(const UianaSettings* UianaSettings)
{
	Settings = UianaSettings;
}

template<class ObjType>
void TBaseImporter<ObjType>::SetSettingsFromJsonProperties(const TSharedPtr<FJsonObject> JsonProps, ObjType* BaseObj)
{
	// Loop through all JSON values (type <FString, TSharedPtr<FJsonValue>>)
	for (auto const& prop : JsonProps.Get()->Values)
	{
		TSharedPtr<FJsonValue> propValue = prop.Value;
		const FName propName = FName(*prop.Key);
		FProperty* objectProp = PropertyAccessUtil::FindPropertyByName(propName, BaseObj->GetClass());
		if (objectProp == nullptr) continue;
		const EJson propType = propValue.Get()->Type;
		if (propType == EJson::Number)
		{
			if (OverrideNumericProp(prop.Key, propValue, objectProp, BaseObj)) continue;
			if (const FFloatProperty* floatProp = CastField<FFloatProperty>(objectProp))
			{
				floatProp->SetPropertyValue_InContainer(BaseObj, prop.Value.Get()->AsNumber());
			}
			else if (const FIntProperty* intProp = CastField<FIntProperty>(objectProp))
			{
				intProp->SetPropertyValue_InContainer(BaseObj, prop.Value.Get()->AsNumber());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to cast %s into numeric prop!"), *prop.Key);	
			}
		}
		else if (propType == EJson::Boolean)
		{
			if (const FBoolProperty* boolProp = CastField<FBoolProperty>(objectProp))
			{
				boolProp->SetPropertyValue_InContainer(BaseObj, prop.Value.Get()->AsBool());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to cast %s into bool prop!"), *prop.Key);
			}
		}
		else if (propType == EJson::Object)
		{
			if (OverrideObjectProp(prop.Key, propValue, objectProp, BaseObj)) continue;
			if (objectProp->GetClass()->GetName().Equals("StructProperty"))
			{
				const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
				if (const FStructProperty* colorValues = CastField<FStructProperty>(objectProp))
				{
					FString OutputString;
					TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
					FJsonSerializer::Serialize(propValue.Get()->AsObject().ToSharedRef(), Writer);
					UScriptStruct* Class = nullptr;
					FString ClassName = objectProp->GetCPPType().TrimChar('F');
					/*
					 * CPPTypes handled:
					 * BodyInstance					Meshes, Blueprints
					 * Vector						Meshes, Lights, Decals, Blueprints
					 * LightmassPrimitiveSettings	Meshes
					 * Rotator						Meshes, Lights, Decals, Blueprints
					 * Box							Meshes
					 * BoxSphereBounds				Meshes
					 * LinearColor					Lights
					 * Color						Lights
					 **/
					/*
					 * Properties handled:
					 * BodyInstance					Meshes, Blueprints
					 * RelativeLocation				Meshes, Lights, Decals, Blueprints
					 * RelativeRotation				Meshes, Lights, Decals, Blueprints
					 * RelativeScale3D				Meshes, Lights, Decals, Blueprints
					 * LightmassSettings			Meshes
					 * BuiltInstanceBounds			Meshes
					 * CacheMeshExtendedBounds		Meshes
					 * FogInscatteringColor			Lights
					 * ModulatedShadowColor			Lights
					 * DecalSize					Decals
					 **/
					UE_LOG(LogTemp, Display, TEXT("Uiana: Setting Actor property %s of type %s for JSON %s"), *prop.Key, *ClassName, *OutputString);
					if (!FPackageName::IsShortPackageName(ClassName))
					{
						Class = FindObject<UScriptStruct>(nullptr, *ClassName);
					}
					else
					{
						Class = FindFirstObject<UScriptStruct>(*ClassName, EFindFirstObjectOptions::None, ELogVerbosity::Warning, TEXT("FEditorClassUtils::GetClassFromString"));
					}
					if(!Class)
					{
						Class = LoadObject<UScriptStruct>(nullptr, *ClassName);
					}
					if (!Class)
					{
						UE_LOG(LogTemp, Display, TEXT("Uiana: Failed to find UScriptStruct for property %s of type %s!"), *prop.Key, *ClassName);
						continue;
					}
					void* structSettingsAddr = objectProp->ContainerPtrToValuePtr<void>(BaseObj);
					FJsonObject* propObj = new FJsonObject();
					TSharedPtr<FJsonObject> propObjPtr = MakeShareable(propObj);
					FJsonObject::Duplicate(obj, propObjPtr);
					// Overrides are not correctly set unless corresponding bOverride flag is set to true
					for (const TTuple<FString, TSharedPtr<FJsonValue>> structVal : obj->Values)
					{
						for (FString overrideVariation : {"bOverride", "bOverride_"})
						{
							FString overridePropName = overrideVariation + structVal.Key;
							if (FProperty* overrideFlagProp = Class->FindPropertyByName(FName(*overridePropName)))
							{
								void* propAddr = overrideFlagProp->ContainerPtrToValuePtr<uint8>(structSettingsAddr);
								if (FBoolProperty* overrideProp = CastField<FBoolProperty>(overrideFlagProp))
								{
									UE_LOG(LogTemp, Display, TEXT("Uiana: Setting override flag for setting %s to true"), *overridePropName);
									overrideProp->SetPropertyValue(propAddr, true);
								}
							}	
						}
						if (obj->HasTypedField<EJson::String>("ShadingModel") && propObjPtr->GetStringField("ShadingModel").Equals("MSM_AresEnvironment"))
						{
							// Custom Valorant-related override, this does not exist in Enum list and makes BasePropertyOverrides fail.
							// TODO: Instead search for String JsonValues and verify the corresponding Enum has the value or not?
							propObjPtr->SetStringField("ShadingModel", "MSM_DefaultLit");
						}
					}
					FText FailureReason;
					if (!FJsonObjectConverter::JsonObjectToUStruct(propObjPtr.ToSharedRef(), Class, structSettingsAddr,
						0, 0, false, &FailureReason)) UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to set %s due to reason %s"), *prop.Key, *FailureReason.ToString());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Failed to cast %s into StructProperty struct!"), *prop.Key);
				}
			}
		}
		else if (propType == EJson::Array)
		{
			if (OverrideArrayProp(prop.Key, propValue, objectProp, BaseObj)) continue;
			// TODO: Add automated creation of array entities!
		}
		else if (propType == EJson::String)
		{
			if (const FEnumProperty* enumProp = CastField<FEnumProperty>(objectProp))
			{
				if (const UEnum* Enum = enumProp->GetEnum())
				{
					FString strValue = propValue->AsString();
					int64 enumValue = Enum->GetValueByName(FName(*strValue), EGetByNameFlags::CheckAuthoredName);
					if (enumValue == INDEX_NONE)
					{
						UE_LOG(LogTemp, Error, TEXT("Uiana: Failed to set EnumProperty %s to value %s"), *prop.Key, *propValue->AsString());
						continue;
					}
					void* structSettingsAddr = objectProp->ContainerPtrToValuePtr<void>(BaseObj);
					void* propAddr = enumProp->ContainerPtrToValuePtr<uint8>(structSettingsAddr);
					enumProp->GetUnderlyingProperty()->SetIntPropertyValue(propAddr, enumValue);
				}
			}
			else if (const FNumericProperty* numericProp = CastField<FNumericProperty>(objectProp))
			{
				if (numericProp->IsEnum())
				{
					if (const UEnum* Enum = numericProp->GetIntPropertyEnum())
					{
						FString strValue = propValue->AsString();
						int64 enumValue = Enum->GetValueByName(FName(*strValue), EGetByNameFlags::CheckAuthoredName);
						if (enumValue == INDEX_NONE)
						{
							UE_LOG(LogTemp, Error, TEXT("Uiana: Failed to set Numeric EnumProperty %s to value %s"), *prop.Key, *propValue->AsString());
							continue;
						}
						void* structSettingsAddr = objectProp->ContainerPtrToValuePtr<void>(BaseObj);
						void* propAddr = numericProp->ContainerPtrToValuePtr<uint8>(structSettingsAddr);
						numericProp->SetIntPropertyValue(propAddr, enumValue);
					}
				}
			}
			else if (const FStrProperty* stringProp = CastField<FStrProperty>(objectProp))
			{
				stringProp->SetPropertyValue_InContainer(BaseObj, propValue->AsString());
			}
		}
		else
		{
			if (prop.Key.Equals("StreamingTextureData")) continue;
			FString type = "None";
			if (propType == EJson::Array) type = "Array";
			if (propType == EJson::Object) type = "Object";
			if (propType == EJson::Boolean) type = "Bool";
			if (propType == EJson::String) type = "String";
			if (propType == EJson::Number) type = "Number";
			if (propType == EJson::Null) type = "Null";
			UE_LOG(LogTemp, Warning, TEXT("Uiana: Unset BP Property %s of type %s with CPP type %s!"), *prop.Key, *type, *objectProp->GetClass()->GetName());
			if (propType == EJson::String)
			{
				UE_LOG(LogTemp, Warning, TEXT("Uiana: String BP Property unset - %s: %s"), *prop.Key, *propValue->AsString());
			}
			/**
			 * Unhandled Properties:
			 * CaptureOffsetComponent		[ObjectProperty]	Lights
			 * UCSModifiedProperties		[ArrayProperty]		Lights
			 * SortedInstances				[ArrayProperty]		Meshes
			 * StreamingTextureData			[ArrayProperty]		Meshes, Blueprints
			 * AssetUserData				[ArrayProperty]		Meshes
			 * ComponentTags				[ArrayProperty]		Meshes
			 * InstanceReorderTable			[ArrayProperty]		Meshes
			 * 
			 */
		}
	}
}

template<class ObjType>
bool TBaseImporter<ObjType>::OverrideNumericProp(const FString JsonPropName, const TSharedPtr<FJsonValue> JsonPropValue,
	const FProperty* ObjectProp, ObjType* BaseObj)
{
	return false;
}

template<class ObjType>
bool TBaseImporter<ObjType>::OverrideObjectProp(const FString JsonPropName, const TSharedPtr<FJsonValue> JsonPropValue,
	const FProperty* ObjectProp, ObjType* BaseObj)
{
	return false;
}

template<class ObjType>
bool TBaseImporter<ObjType>::OverrideArrayProp(const FString JsonPropName, const TSharedPtr<FJsonValue> JsonPropValue,
	const FProperty* ObjectProp, ObjType* BaseObj)
{
	UE_LOG(LogTemp, Error, TEXT("Uiana: Called BaseImporter OverrideArrayProp, should not be possible!"));
	return false;
}

// Explicit template instantiation
template class TBaseImporter<UActorComponent>;
template class TBaseImporter<UMaterialInstanceConstant>;