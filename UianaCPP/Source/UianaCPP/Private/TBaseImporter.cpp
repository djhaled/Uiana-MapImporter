#include "TBaseImporter.h"

#include "EditorClassUtils.h"
#include "UianaHelpers.h"

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
	for (auto const& JsonProp : JsonProps.Get()->Values)
	{
		TSharedPtr<FJsonValue> PropValue = JsonProp.Value;
		const FName PropName = FName(*JsonProp.Key);
		FProperty* ObjectProp = PropertyAccessUtil::FindPropertyByName(PropName, BaseObj->GetClass());
		if (ObjectProp == nullptr) continue;
		const EJson PropType = PropValue.Get()->Type;
		if (PropType == EJson::Number)
		{
			if (OverrideNumericProp(JsonProp.Key, PropValue, ObjectProp, BaseObj)) continue;
			if (const FFloatProperty* FloatProp = CastField<FFloatProperty>(ObjectProp))
			{
				FloatProp->SetPropertyValue_InContainer(BaseObj, JsonProp.Value.Get()->AsNumber());
			}
			else if (const FIntProperty* IntProp = CastField<FIntProperty>(ObjectProp))
			{
				IntProp->SetPropertyValue_InContainer(BaseObj, JsonProp.Value.Get()->AsNumber());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to cast %s into numeric prop!"), *JsonProp.Key);	
			}
		}
		else if (PropType == EJson::Boolean)
		{
			if (const FBoolProperty* BoolProp = CastField<FBoolProperty>(ObjectProp))
			{
				BoolProp->SetPropertyValue_InContainer(BaseObj, JsonProp.Value.Get()->AsBool());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to cast %s into bool prop!"), *JsonProp.Key);
			}
		}
		else if (PropType == EJson::Object)
		{
			if (OverrideObjectProp(JsonProp.Key, PropValue, ObjectProp, BaseObj)) continue;
			if (ObjectProp->GetClass()->GetName().Equals("StructProperty"))
			{
				const TSharedPtr<FJsonObject> Obj = PropValue.Get()->AsObject();
				if (const FStructProperty* ColorValues = CastField<FStructProperty>(ObjectProp))
				{
					FString OutputString;
					TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
					FJsonSerializer::Serialize(PropValue.Get()->AsObject().ToSharedRef(), Writer);
					UScriptStruct* Class = nullptr;
					FString ClassName = ObjectProp->GetCPPType();
					ClassName.RemoveFromStart("F", ESearchCase::CaseSensitive);
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
					UE_LOG(LogTemp, Display, TEXT("Uiana: Setting Actor property %s of type %s for JSON %s"), *JsonProp.Key, *ClassName, *OutputString);
					if (!FPackageName::IsShortPackageName(ClassName))
					{
						Class = FindObject<UScriptStruct>(nullptr, *ClassName);
					}
					else
					{
						
						Class = FindFirstObject<UScriptStruct>(*ClassName, EFindFirstObjectOptions::None, ELogVerbosity::Warning, TEXT("FEditorClassUtils::GetClassFromString"));
					}
					// TODO: Fix why cannot find UScriptStruct!
					if(!Class)
					{
						Class = LoadObject<UScriptStruct>(nullptr, *ClassName);
					}
					if (!Class)
					{
						UE_LOG(LogTemp, Display, TEXT("Uiana: Failed to find UScriptStruct for property %s of type %s!"), *JsonProp.Key, *ClassName);
						continue;
					}
					void* StructSettingsAddr = ObjectProp->ContainerPtrToValuePtr<void>(BaseObj);
					FJsonObject* PropObj = new FJsonObject();
					TSharedPtr<FJsonObject> PropObjPtr = MakeShareable(PropObj);
					UianaHelpers::DuplicateJsonObj(Obj, PropObjPtr);
					// Overrides are not correctly set unless corresponding bOverride flag is set to true
					for (const TTuple<FString, TSharedPtr<FJsonValue>> StructVal : Obj->Values)
					{
						for (FString OverrideVariation : {"bOverride", "bOverride_"})
						{
							FString OverridePropName = OverrideVariation + StructVal.Key;
							if (FProperty* overrideFlagProp = Class->FindPropertyByName(FName(*OverridePropName)))
							{
								void* PropAddr = overrideFlagProp->ContainerPtrToValuePtr<uint8>(StructSettingsAddr);
								if (FBoolProperty* OverrideProp = CastField<FBoolProperty>(overrideFlagProp))
								{
									UE_LOG(LogTemp, Display, TEXT("Uiana: Setting override flag for setting %s to true"), *OverridePropName);
									OverrideProp->SetPropertyValue(PropAddr, true);
								}
							}	
						}
						if (Obj->HasTypedField<EJson::String>("ShadingModel") && PropObjPtr->GetStringField("ShadingModel").Equals("MSM_AresEnvironment"))
						{
							// Custom Valorant-related override, this does not exist in Enum list and makes BasePropertyOverrides fail.
							// TODO: Instead search for String JsonValues and verify the corresponding Enum has the value or not?
							PropObjPtr->SetStringField("ShadingModel", "MSM_DefaultLit");
						}
					}
#if ENGINE_MAJOR_VERSION == 5
					FText FailureReason;
					if (!FJsonObjectConverter::JsonObjectToUStruct(PropObjPtr.ToSharedRef(), Class, StructSettingsAddr,
						0, 0, false, &FailureReason)) UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to set %s due to reason %s"), *JsonProp.Key, *FailureReason.ToString());
#else
					FJsonObjectConverter::JsonObjectToUStruct(PropObjPtr.ToSharedRef(), Class, StructSettingsAddr, 0, 0);
#endif
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Failed to cast %s into StructProperty struct!"), *JsonProp.Key);
				}
			}
		}
		else if (PropType == EJson::Array)
		{
			if (OverrideArrayProp(JsonProp.Key, PropValue, ObjectProp, BaseObj)) continue;
			// TODO: Add automated creation of array entities!
		}
		else if (PropType == EJson::String)
		{
			if (const FEnumProperty* EnumProp = CastField<FEnumProperty>(ObjectProp))
			{
				if (const UEnum* Enum = EnumProp->GetEnum())
				{
					FString StrValue = PropValue->AsString();
					int64 EnumValue = Enum->GetValueByName(FName(*StrValue), EGetByNameFlags::CheckAuthoredName);
					if (EnumValue == INDEX_NONE)
					{
						UE_LOG(LogTemp, Error, TEXT("Uiana: Failed to set EnumProperty %s to value %s"), *JsonProp.Key, *PropValue->AsString());
						continue;
					}
					void* StructSettingsAddr = ObjectProp->ContainerPtrToValuePtr<void>(BaseObj);
					void* PropAddr = EnumProp->ContainerPtrToValuePtr<uint8>(StructSettingsAddr);
					EnumProp->GetUnderlyingProperty()->SetIntPropertyValue(PropAddr, EnumValue);
				}
			}
			else if (const FNumericProperty* NumericProp = CastField<FNumericProperty>(ObjectProp))
			{
				if (NumericProp->IsEnum())
				{
					if (const UEnum* Enum = NumericProp->GetIntPropertyEnum())
					{
						FString StrValue = PropValue->AsString();
						int64 EnumValue = Enum->GetValueByName(FName(*StrValue), EGetByNameFlags::CheckAuthoredName);
						if (EnumValue == INDEX_NONE)
						{
							UE_LOG(LogTemp, Error, TEXT("Uiana: Failed to set Numeric EnumProperty %s to value %s"), *JsonProp.Key, *PropValue->AsString());
							continue;
						}
						void* StructSettingsAddr = ObjectProp->ContainerPtrToValuePtr<void>(BaseObj);
						void* PropAddr = NumericProp->ContainerPtrToValuePtr<uint8>(StructSettingsAddr);
						NumericProp->SetIntPropertyValue(PropAddr, EnumValue);
					}
				}
			}
			else if (const FStrProperty* StringProp = CastField<FStrProperty>(ObjectProp))
			{
				StringProp->SetPropertyValue_InContainer(BaseObj, PropValue->AsString());
			}
		}
		else
		{
			FString Type = "None";
			if (PropType == EJson::Array) Type = "Array";
			if (PropType == EJson::Object) Type = "Object";
			if (PropType == EJson::Boolean) Type = "Bool";
			if (PropType == EJson::String) Type = "String";
			if (PropType == EJson::Number) Type = "Number";
			if (PropType == EJson::Null) Type = "Null";
			UE_LOG(LogTemp, Warning, TEXT("Uiana: Unset BP Property %s of type %s with CPP type %s!"), *JsonProp.Key, *Type, *ObjectProp->GetClass()->GetName());
			if (PropType == EJson::String)
			{
				UE_LOG(LogTemp, Warning, TEXT("Uiana: String BP Property unset - %s: %s"), *JsonProp.Key, *PropValue->AsString());
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