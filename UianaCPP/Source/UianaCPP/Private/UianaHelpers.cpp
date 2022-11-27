#include "UianaHelpers.h"

#include "JsonObjectConverter.h"
#include "ObjectEditorUtils.h"
#include "Animation/Rig.h"
#include "Misc/FileHelper.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UianaHelpers::CreateFolder(FDirectoryPath& FolderPath, FString Root, FString Extension)
{
	FolderPath.Path = FPaths::Combine(Root, Extension).Replace(TEXT("\\"), TEXT("/"));
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	if (FileManager.DirectoryExists(*(FolderPath.Path)))
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Not creating directory %s since it already exists."), *(FolderPath.Path));
	}
	else if (!FileManager.CreateDirectory(*(FolderPath.Path)))
	{
		UE_LOG(LogTemp, Error, TEXT("Uiana: Failed to create folder %s"), *(FolderPath.Path));
	}
}

TSharedPtr<FJsonObject> UianaHelpers::ParseJson(FString InputStr)
{
	TSharedPtr<FJsonObject> JsonParsed = MakeShareable(new FJsonObject());
	const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(InputStr);
	if (FJsonSerializer::Deserialize(JsonReader, JsonParsed) && JsonParsed.IsValid())
	{
		return JsonParsed;
	}
	return nullptr;
}

void UianaHelpers::SaveJson(const TArray<TSharedPtr<FJsonValue>> json, const FString path)
{
	FString OutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(json, Writer);
	IFileManager& FileManager = IFileManager::Get();
	if (FPaths::ValidatePath(path) && FPaths::FileExists(path))
	{
		FileManager.Delete(*path);
	}
	FFileHelper::SaveStringToFile(OutputString, *path, FFileHelper::EEncodingOptions::AutoDetect, &FileManager);
}

void UianaHelpers::AddAllAssetPath(TArray<FString> &allAssets, const TArray<FString> assetsToAdd)
{
	for (int i = 0; i < assetsToAdd.Num(); i++)
	{
		FString firstDir, secondDir, temp, remainingDirs;
		assetsToAdd[i].Split("\\", &firstDir, &temp);
		temp.Split("\\", &secondDir, &remainingDirs);
		allAssets.AddUnique(FPaths::Combine(firstDir.Equals("ShooterGame") ? "Game" : firstDir, secondDir.Equals("Content") ? "" : secondDir, remainingDirs));
	}
}

void UianaHelpers::AddPrefixPath(const FDirectoryPath path, TArray<FString> &suffixes)
{
	for (int i = 0; i < suffixes.Num(); i++)
	{
		suffixes[i] = FPaths::Combine(path.Path, suffixes[i]);
	}
}

template <class PropType, class CppType>
bool UianaHelpers::SetStructProperty(void* data, const FProperty* objectProp, CppType value)
{
	if (const PropType* childProp = CastField<PropType>(objectProp))
	{
		childProp->SetPropertyValue(data, value);
		return true;
	}
	return false;
}

template<class PropType, class CppType>
bool UianaHelpers::SetStructPropertiesFromJson(void* data, const FProperty* objectProp, const TSharedPtr<FJsonObject> jsonObj, const TArray<FName> jsonProps)
{
	bool success = false;
	if (const FStructProperty* childProp = CastField<FStructProperty>(objectProp))
	{
		success = true;
		UScriptStruct* scriptStruct = childProp->Struct;
		for (const FName propName : jsonProps)
		{
			double val = jsonObj->GetNumberField(propName.ToString());
			FProperty* childVectorPropR = scriptStruct->FindPropertyByName(propName);
			success = success && UianaHelpers::SetStructProperty<PropType, CppType>(data, childVectorPropR, val);
		}
	}
	return success;
}

TEnumAsByte<EMaterialShadingModel> UianaHelpers::ParseShadingModel(const FString model)
{
	if (model.Equals("MSM_Unlit")) return EMaterialShadingModel::MSM_Unlit;
	else if (model.Equals("MSM_DefaultLit")) return EMaterialShadingModel::MSM_DefaultLit;
	else if (model.Equals("MSM_Subsurface")) return EMaterialShadingModel::MSM_Subsurface;
	else if (model.Equals("MSM_PreintegratedSkin")) return EMaterialShadingModel::MSM_PreintegratedSkin;
	else if (model.Equals("MSM_ClearCoat")) return EMaterialShadingModel::MSM_ClearCoat;
	else if (model.Equals("MSM_SubsurfaceProfile")) return EMaterialShadingModel::MSM_SubsurfaceProfile;
	else if (model.Equals("MSM_TwoSidedFoliage")) return EMaterialShadingModel::MSM_TwoSidedFoliage;
	else if (model.Equals("MSM_Hair")) return EMaterialShadingModel::MSM_Hair;
	else if (model.Equals("MSM_Cloth")) return EMaterialShadingModel::MSM_Cloth;
	else if (model.Equals("MSM_Eye")) return EMaterialShadingModel::MSM_Eye;
	else if (model.Equals("MSM_SingleLayerWater")) return EMaterialShadingModel::MSM_SingleLayerWater;
	else if (model.Equals("MSM_ThinTranslucent")) return EMaterialShadingModel::MSM_ThinTranslucent;
	else if (model.Equals("MSM_Strata")) return EMaterialShadingModel::MSM_Strata;
	else if (model.Equals("MSM_MAX")) return EMaterialShadingModel::MSM_MAX;
	return EMaterialShadingModel::MSM_DefaultLit;
}

TEnumAsByte<EBlendMode> UianaHelpers::ParseBlendMode(const FString mode)
{
	if (mode.Equals("BLEND_Opaque")) return EBlendMode::BLEND_Opaque;
	else if (mode.Equals("BLEND_Masked")) return EBlendMode::BLEND_Masked;
	else if (mode.Equals("BLEND_Translucent")) return EBlendMode::BLEND_Translucent;
	else if (mode.Equals("BLEND_Additive")) return EBlendMode::BLEND_Additive;
	else if (mode.Equals("BLEND_Modulate")) return EBlendMode::BLEND_Modulate;
	else if (mode.Equals("BLEND_AlphaComposite")) return EBlendMode::BLEND_AlphaComposite;
	else if (mode.Equals("BLEND_AlphaHoldout")) return EBlendMode::BLEND_AlphaHoldout;
	else if (mode.Equals("BLEND_MAX")) return EBlendMode::BLEND_MAX;
	return EBlendMode::BLEND_Opaque;
}

TEnumAsByte<ECollisionTraceFlag> UianaHelpers::ParseCollisionTrace(const FString flag)
{
	if (flag.Equals("CTF_UseDefault")) return ECollisionTraceFlag::CTF_UseDefault;
	else if (flag.Equals("CTF_UseSimpleAndComplex")) return ECollisionTraceFlag::CTF_UseSimpleAndComplex;
	else if (flag.Equals("CTF_UseSimpleAsComplex")) return ECollisionTraceFlag::CTF_UseSimpleAsComplex;
	else if (flag.Equals("CTF_UseComplexAsSimple")) return ECollisionTraceFlag::CTF_UseComplexAsSimple;
	else if (flag.Equals("CTF_MAX")) return ECollisionTraceFlag::CTF_MAX;
	return ECollisionTraceFlag::CTF_UseDefault;
}

UianaHelpers::ObjectType UianaHelpers::ParseObjectType(const FString objType)
{
	if (MeshRelatedObjects.Contains(objType)) return Mesh;
	else if (LightRelatedObjects.Contains(objType)) return Light;
	else if (DecalRelatedObjects.Contains(objType)) return Decal;
	else if (BlueprintRelatedObjects.Contains(objType)) return Blueprint;
	return Unknown;
}

void UianaHelpers::GetTransformComponent(const TSharedPtr<FJsonObject> comp, FTransform* transform)
{
	// Get transform
	TSharedPtr<FJsonObject> transformData;
	FVector location = FVector::ZeroVector;
	FRotator rotation = FRotator::ZeroRotator;
	FVector scale = FVector::OneVector;
	if (comp->HasField("TransformData"))
	{
		transformData = comp->GetObjectField("TransformData");
	}
	if (JsonObjContainsFields(comp, {"RelativeLocation", "OffsetLocation", "Translation"}))
	{
		const TSharedPtr<FJsonObject> locationData = transformData != nullptr ? transformData->GetObjectField("Translation") : comp->GetObjectField("RelativeLocation");
		location.X = locationData->GetNumberField("X");
		location.Y = locationData->GetNumberField("Y");
		location.Z = locationData->GetNumberField("Z");
	}
	if (JsonObjContainsFields(comp, {"RelativeScale3D", "Scale3D"}))
	{
		const TSharedPtr<FJsonObject> scaleData = transformData != nullptr ? transformData->GetObjectField("Scale3D") : comp->GetObjectField("RelativeScale3D");
		scale.X = scaleData->GetNumberField("X");
		scale.Y = scaleData->GetNumberField("Y");
		scale.Z = scaleData->GetNumberField("Z");
	}
	if (JsonObjContainsFields(comp, {"RelativeRotation", "Rotation"}))
	{
		if (transformData != nullptr)
		{
			const TSharedPtr<FJsonObject> rotationData = transformData->GetObjectField("Rotation");
			FQuat quat;
			quat.X = rotationData->GetNumberField("X");
			quat.Y = rotationData->GetNumberField("Y");
			quat.Z = rotationData->GetNumberField("Z");
			quat.W = rotationData->GetNumberField("W");
			*transform = FTransform(quat, location, scale);
			// FObjectEditorUtils::SetPropertyValue(&transform, FName("Rotation"), quat);
		}
		else
		{
			const TSharedPtr<FJsonObject> rotationData = comp->GetObjectField("Rotation");
			rotation.Roll = rotationData->GetNumberField("Roll");
			rotation.Pitch = rotationData->GetNumberField("Pitch");
			rotation.Yaw = rotationData->GetNumberField("Yaw");
			*transform = FTransform(rotation, location, scale);
		}
	}
	else
	{
		*transform = FTransform(rotation, location, scale);
	}
}

void UianaHelpers::GetSceneTransformComponent(const TSharedPtr<FJsonObject> comp, FTransform* transform)
{
	FVector location = FVector::ZeroVector;
	FRotator rotation = FRotator::ZeroRotator;
	FVector scale = FVector::OneVector;
	if (comp->HasField("SceneAttachRelativeLocation"))
	{
		location.X = comp->GetObjectField("SceneAttachRelativeLocation")->GetNumberField("X");
		location.Y = comp->GetObjectField("SceneAttachRelativeLocation")->GetNumberField("Y");
		location.Z = comp->GetObjectField("SceneAttachRelativeLocation")->GetNumberField("Z");
	}
	if (comp->HasField("SceneAttachRelativeRotation"))
	{
		rotation.Roll = comp->GetObjectField("SceneAttachRelativeRotation")->GetNumberField("Roll");
		rotation.Pitch = comp->GetObjectField("SceneAttachRelativeRotation")->GetNumberField("Pitch");
		rotation.Yaw = comp->GetObjectField("SceneAttachRelativeRotation")->GetNumberField("Yaw");
	}
	if (comp->HasField("SceneAttachRelativeScale3D"))
	{
		scale.X = comp->GetObjectField("SceneAttachRelativeScale3D")->GetNumberField("X");
		scale.Y = comp->GetObjectField("SceneAttachRelativeScale3D")->GetNumberField("Y");
		scale.Z = comp->GetObjectField("SceneAttachRelativeScale3D")->GetNumberField("Z");
	}
	*transform = FTransform(rotation, location, scale);
}

bool UianaHelpers::HasTransformComponent(const TSharedPtr<FJsonObject> comp)
{
	const TSet<FString> transformCompNames = {"RelativeLocation", "SceneAttachRelativeLocation", "SceneAttachRelativeRotation", "SceneAttachRelativeScale3D", "RelativeRotation", "RelativeScale3D"};
	return JsonObjContainsFields(comp, transformCompNames); // TODO: Reduce this small function
}

bool UianaHelpers::JsonObjContainsFields(const TSharedPtr<FJsonObject> obj, const TSet<FString> fields)
{
	for (auto field : fields)
	{
		if (obj->HasField(field)) return true;
	}
	return false;
}

bool operator == (TSharedPtr<FJsonValue, ESPMode::ThreadSafe> arrayItem, FString item)
{
	return arrayItem->AsObject()->GetStringField("Name").Equals(item);
}

template <class PropType>
bool UianaHelpers::SetActorProperty(UActorComponent* component, const FString propName, PropType propVal)
{
	FProperty* relativeLocationProp = UStaticMesh::StaticClass()->FindPropertyByName(FName(*propName));
	PropType* locationAddr = relativeLocationProp->ContainerPtrToValuePtr<PropType>(component);
	if (locationAddr == nullptr) return false;
	component->PreEditChange(relativeLocationProp);
	*locationAddr = propVal;
	FPropertyChangedEvent locationChangedEvent(relativeLocationProp);
	component->PostEditChangeProperty(locationChangedEvent);
	return true;
}