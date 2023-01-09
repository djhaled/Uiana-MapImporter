#include "UianaHelpers.h"

#include "EditorSubsystemModule.h"
#include "JsonObjectConverter.h"
#include "ObjectEditorUtils.h"
#include "Animation/Rig.h"
#include "Misc/FileHelper.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/PropertyAccessUtil.h"

const TSet<FString> UianaHelpers::LightRelatedObjects = {"PointLightComponent", "PostProcessVolume", "PrecomputedVisibilityVolume", "CullDistanceVolume",
			  "RectLightComponent", "LightmassCharacterIndirectDetailVolume", "SpotLightComponent", "SkyLightComponent",
			  "LightmassImportanceVolume", "SceneCaptureComponentCube", "SphereReflectionCaptureComponent",
			  "DirectionalLightComponent", "ExponentialHeightFogComponent", "LightmassPortalComponent"};
const TSet<FString> UianaHelpers::MeshRelatedObjects = {"StaticMeshComponent", "InstancedStaticMeshComponent", "HierarchicalInstancedStaticMeshComponent"};
const TSet<FString> UianaHelpers::DecalRelatedObjects = {"DecalComponent"};
const TSet<FString> UianaHelpers::BlueprintRelatedObjects = {"SceneComponent"};

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

void UianaHelpers::SaveJson(const TArray<TSharedPtr<FJsonValue>> JsonObj, const FString Path)
{
	FString OutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObj, Writer);
	IFileManager& FileManager = IFileManager::Get();
	if (FPaths::ValidatePath(Path) && FPaths::FileExists(Path))
	{
		FileManager.Delete(*Path);
	}
	FFileHelper::SaveStringToFile(OutputString, *Path, FFileHelper::EEncodingOptions::AutoDetect, &FileManager);
}

void UianaHelpers::AddAllAssetPath(TArray<FString> &AllAssets, const TArray<FString> AssetsToAdd)
{
	for (int i = 0; i < AssetsToAdd.Num(); i++)
	{
		FString firstDir, secondDir, temp, remainingDirs;
		AssetsToAdd[i].Split("\\", &firstDir, &temp);
		temp.Split("\\", &secondDir, &remainingDirs);
		AllAssets.AddUnique(FPaths::Combine(firstDir.Equals("ShooterGame") ? "Game" : firstDir, secondDir.Equals("Content") ? "" : secondDir, remainingDirs));
	}
}

void UianaHelpers::AddPrefixPath(const FDirectoryPath Path, TArray<FString> &Suffixes)
{
	for (int i = 0; i < Suffixes.Num(); i++)
	{
		Suffixes[i] = FPaths::Combine(Path.Path, Suffixes[i]);
	}
}

TEnumAsByte<ECollisionTraceFlag> UianaHelpers::ParseCollisionTrace(const FString Flag)
{
	if (Flag.Equals("CTF_UseDefault")) return ECollisionTraceFlag::CTF_UseDefault;
	else if (Flag.Equals("CTF_UseSimpleAndComplex")) return ECollisionTraceFlag::CTF_UseSimpleAndComplex;
	else if (Flag.Equals("CTF_UseSimpleAsComplex")) return ECollisionTraceFlag::CTF_UseSimpleAsComplex;
	else if (Flag.Equals("CTF_UseComplexAsSimple")) return ECollisionTraceFlag::CTF_UseComplexAsSimple;
	else if (Flag.Equals("CTF_MAX")) return ECollisionTraceFlag::CTF_MAX;
	return ECollisionTraceFlag::CTF_UseDefault;
}

UianaHelpers::EObjectType UianaHelpers::ParseObjectType(const FString ObjType)
{
	if (MeshRelatedObjects.Contains(ObjType)) return Mesh;
	else if (LightRelatedObjects.Contains(ObjType)) return Light;
	else if (DecalRelatedObjects.Contains(ObjType)) return Decal;
	else if (BlueprintRelatedObjects.Contains(ObjType)) return Blueprint;
	return Unknown;
}

FTransform UianaHelpers::GetTransformComponent(const TSharedPtr<FJsonObject> Comp)
{
	// Get transform
	bool directTransform = false;
	TSharedPtr<FJsonObject> transformData = Comp;
	FVector location = FVector::ZeroVector;
	FRotator rotation = FRotator::ZeroRotator;
	FVector scale = FVector::OneVector;
	if (Comp->HasField("TransformData"))
	{
		transformData = Comp->GetObjectField("TransformData");
		directTransform = true;
	}
	if (JsonObjContainsFields(transformData, {"RelativeLocation", "OffsetLocation", "Translation"}))
	{
		const TSharedPtr<FJsonObject> locationData = directTransform ? transformData->GetObjectField("Translation") : transformData->GetObjectField("RelativeLocation");
		location.X = locationData->GetNumberField("X");
		location.Y = locationData->GetNumberField("Y");
		location.Z = locationData->GetNumberField("Z");
	}
	if (JsonObjContainsFields(transformData, {"RelativeScale3D", "Scale3D"}))
	{
		const TSharedPtr<FJsonObject> scaleData = directTransform ? transformData->GetObjectField("Scale3D") : transformData->GetObjectField("RelativeScale3D");
		scale.X = scaleData->GetNumberField("X");
		scale.Y = scaleData->GetNumberField("Y");
		scale.Z = scaleData->GetNumberField("Z");
	}
	if (JsonObjContainsFields(transformData, {"RelativeRotation", "Rotation"}))
	{
		if (directTransform)
		{
			const TSharedPtr<FJsonObject> rotationData = transformData->GetObjectField("Rotation");
			FQuat quat;
			quat.X = rotationData->GetNumberField("X");
			quat.Y = rotationData->GetNumberField("Y");
			quat.Z = rotationData->GetNumberField("Z");
			quat.W = rotationData->GetNumberField("W");
			return FTransform(quat, location, scale);
		}
		else
		{
			const TSharedPtr<FJsonObject> rotationData = transformData->GetObjectField("RelativeRotation");
			rotation.Roll = rotationData->GetNumberField("Roll");
			rotation.Pitch = rotationData->GetNumberField("Pitch");
			rotation.Yaw = rotationData->GetNumberField("Yaw");
			return FTransform(rotation, location, scale);
		}
	}
	return FTransform(rotation, location, scale);
}

FTransform UianaHelpers::GetSceneTransformComponent(const TSharedPtr<FJsonObject> Comp)
{
	FVector location = FVector::ZeroVector;
	FRotator rotation = FRotator::ZeroRotator;
	FVector scale = FVector::OneVector;
	FString OutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Comp.ToSharedRef(), Writer);
	if (Comp->HasField("SceneAttachRelativeLocation"))
	{
		location.X = Comp->GetObjectField("SceneAttachRelativeLocation")->GetNumberField("X");
		location.Y = Comp->GetObjectField("SceneAttachRelativeLocation")->GetNumberField("Y");
		location.Z = Comp->GetObjectField("SceneAttachRelativeLocation")->GetNumberField("Z");
	}
	if (Comp->HasField("SceneAttachRelativeRotation"))
	{
		rotation.Roll = Comp->GetObjectField("SceneAttachRelativeRotation")->GetNumberField("Roll");
		rotation.Pitch = Comp->GetObjectField("SceneAttachRelativeRotation")->GetNumberField("Pitch");
		rotation.Yaw = Comp->GetObjectField("SceneAttachRelativeRotation")->GetNumberField("Yaw");
	}
	if (Comp->HasField("SceneAttachRelativeScale3D"))
	{
		scale.X = Comp->GetObjectField("SceneAttachRelativeScale3D")->GetNumberField("X");
		scale.Y = Comp->GetObjectField("SceneAttachRelativeScale3D")->GetNumberField("Y");
		scale.Z = Comp->GetObjectField("SceneAttachRelativeScale3D")->GetNumberField("Z");
	}
	return FTransform(rotation, location, scale);
}

bool UianaHelpers::HasTransformComponent(const TSharedPtr<FJsonObject> Comp)
{
	const TSet<FString> transformCompNames = {"RelativeLocation", "SceneAttachRelativeLocation", "SceneAttachRelativeRotation", "SceneAttachRelativeScale3D", "RelativeRotation", "RelativeScale3D"};
	return JsonObjContainsFields(Comp, transformCompNames); // TODO: Reduce this small function
}

bool UianaHelpers::JsonObjContainsFields(const TSharedPtr<FJsonObject> Obj, const TSet<FString> Fields)
{
	for (auto field : Fields)
	{
		if (Obj->HasField(field)) return true;
	}
	return false;
}

bool operator == (TSharedPtr<FJsonValue> arrayItem, FString item)
{
	return arrayItem->AsObject()->GetStringField("Name").Equals(item);
}

template <class PropType>
bool UianaHelpers::SetActorProperty(UClass* ActorClass, UObject* Component, const FString PropName, PropType PropVal)
{
	FProperty* relativeLocationProp = PropertyAccessUtil::FindPropertyByName(FName(*PropName), ActorClass);
	PropType* locationAddr = relativeLocationProp->ContainerPtrToValuePtr<PropType>(Component);
	if (locationAddr == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to set property %s"), *PropName);
		return false;
	}
	Component->PreEditChange(relativeLocationProp);
	*locationAddr = PropVal;
	FPropertyChangedEvent locationChangedEvent(relativeLocationProp);
	Component->PostEditChangeProperty(locationChangedEvent);
	return true;
}

// Necessary for UE4 compatilibity
void UianaHelpers::DuplicateJsonObj(const TSharedPtr<FJsonObject>& Source, TSharedPtr<FJsonObject>& Dest)
{
	if (Source && Dest)
	{
		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Source->Values)
		{
			Dest->SetField(Pair.Key, DuplicateJsonValue(Pair.Value));
		}
	}
}

void UianaHelpers::DuplicateJsonArray(const TArray<TSharedPtr<FJsonValue>>& Source, TArray<TSharedPtr<FJsonValue>>& Dest)
{
	for (const TSharedPtr<FJsonValue>& Value : Source)
	{
		Dest.Add(DuplicateJsonValue(Value));
	} 
}

TSharedPtr<FJsonValue> UianaHelpers::DuplicateJsonValue(const TSharedPtr<FJsonValue>& Src)
{
	switch (Src->Type)
	{
	case EJson::Boolean:
		{
			bool BoolValue;
			if (Src->TryGetBool(BoolValue))
			{
				return MakeShared<FJsonValueBoolean>(BoolValue);
			}
		}
	case EJson::Number:
		{
			double NumberValue;
			if (Src->TryGetNumber(NumberValue))
			{
				return MakeShared<FJsonValueNumber>(NumberValue);
			}
		}
	case EJson::String:
		{
			FString StringValue;
			if (Src->TryGetString(StringValue))
			{
				return MakeShared<FJsonValueString>(StringValue);
			}
		}
	case EJson::Object:
		{
			const TSharedPtr<FJsonObject>* ObjectValue;
			if (Src->TryGetObject(ObjectValue))
			{
				TSharedPtr<FJsonObject> NewObject = MakeShared<FJsonObject>();
				DuplicateJsonObj(*ObjectValue, NewObject);
				return MakeShared<FJsonValueObject>(NewObject);
			}
		}
	case EJson::Array:
		{
			const TArray<TSharedPtr<FJsonValue>>* ArrayValue;
			if (Src->TryGetArray(ArrayValue))
			{
				TArray<TSharedPtr<FJsonValue>> NewArray;
				DuplicateJsonArray(*ArrayValue, NewArray);

				return MakeShared<FJsonValueArray>(NewArray);
			}
		}
	}

	return TSharedPtr<FJsonValue>();
}