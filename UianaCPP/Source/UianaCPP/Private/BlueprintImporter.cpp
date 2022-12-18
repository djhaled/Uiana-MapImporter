// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintImporter.h"

#include "AssetToolsModule.h"
#include "BPFL.h"
#include "EditorAssetLibrary.h"
#include "EditorClassUtils.h"
#include "EditorLevelLibrary.h"
#include "JsonObjectConverter.h"
#include "MaterialImporter.h"
#include "UianaHelpers.h"
#include "Factories/BlueprintFactory.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/PropertyAccessUtil.h"

BlueprintImporter::BlueprintImporter()
{
	Settings = nullptr;
}

BlueprintImporter::BlueprintImporter(const UUianaSettings* UianaSettings)
{
	Settings = UianaSettings;
}


void BlueprintImporter::CreateBlueprints(const UUianaSettings* UianaSettings, const TArray<FString> bpPaths)
{
	Settings = UianaSettings;
	TArray<FString> sceneRoots, childNodes;
	TArray<TSharedPtr<FJsonValue>> newJsons, gameObjs;
	for (const FString bpPath : bpPaths)
	{
		UE_LOG(LogTemp, Display, TEXT("Uiana: Importing BP at path: %s"), *bpPath);
		const TArray<FString> blacklistedBPs = {"SoundBarrier", "SpawnBarrierProjectile", "BP_UnwalkableBlockingVolumeCylinder",
				   "BP_StuckPickupVolume", "BP_BlockingVolume", "TargetingBlockingVolume_Box", "directional_look_up" };
		const FString bpName = FPaths::GetBaseFilename(bpPath);
		if (blacklistedBPs.Contains(bpName)) continue;
		
		// Reduce BP JSON
		FString bpJsonStr;
		FFileHelper::LoadFileToString(bpJsonStr, *bpPath);
		TArray<TSharedPtr<FJsonValue>> bpRaws, bpFiltereds;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(bpJsonStr);
		if (!FJsonSerializer::Deserialize(JsonReader, bpRaws) || !bpRaws[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize blueprint %s"), *bpPath);
			continue;
		}
		for (const TSharedPtr<FJsonValue> bp : bpRaws)
		{
			const TSharedPtr<FJsonObject> bpObj = bp->AsObject();
			if (!bpObj->HasField("Properties")) continue;
			const TSharedPtr<FJsonObject> bpProps = bpObj->GetObjectField("Properties");
			const FString name = bpObj->GetStringField("Name");
			const FString type = bpObj->GetStringField("Type");
			if (type.Equals("SimpleConstructionScript"))
			{
				const FString sceneRoot = bpProps->GetObjectField("DefaultSceneRootNode")->GetStringField("ObjectName");
				FString temp, root;
				sceneRoot.Split(TEXT(":"), &temp, &root, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
				sceneRoots.Add(root);
			}
			if (name.Contains("Node"))
			{
				const FString objName = bpProps->GetObjectField("ComponentTemplate")->GetStringField("ObjectName");
				FString temp, nodeName;
				objName.Split(TEXT(":"), &temp, &nodeName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
				TSharedPtr<FJsonObject> nodeComponent;
				for (const TSharedPtr<FJsonValue> componentBP : bpRaws)
				{
					if (componentBP->AsObject()->HasField("Name"))
					{
						nodeComponent = componentBP->AsObject();
						break;
					}
				}
				if (nodeComponent.IsValid() && nodeComponent->HasField("Properties")) // TODO: CAUSE FOR CRASH - nodeComponent is null and causes crash
					bpProps->SetObjectField("CompProps", nodeComponent->GetObjectField("Properties"));
				newJsons.Add(bp);
				if (bpProps->HasField("ChildNodes"))
				{
					for (const TSharedPtr<FJsonValue> childNode : bpProps->GetArrayField("ChildNodes"))
					{
						FString temp1, childName;
						const FString childNameRaw = childNode.Get()->AsObject()->GetStringField("ObjectName");
						childNameRaw.Split(TEXT("."), &temp1, &childName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
						childNodes.Add(childName);
					}
				}
			}
			if (name.Equals("GameObjectMesh"))
			{
				gameObjs.Add(bp);
			}
		}
		// FJsonObject bpJson = FJsonObject();
		// bpJson.SetArrayField("Nodes", newJsons);
		// bpJson.SetArrayField("SceneRoot", sceneRoots);
		// bpJson.SetArrayField("GameObjects", gameObjs);
		// bpJson.SetArrayField("ChildNodes", childNodes);

		// Create BP
		UE_LOG(LogTemp, Display, TEXT("Uiana: Checking if BP %s exists"), *bpName);
		UBlueprint* bpActor = Cast<UBlueprint>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Blueprints/" + bpName));
		if (bpActor != nullptr) continue;
		UE_LOG(LogTemp, Display, TEXT("Uiana: Creating BP %s"), *bpName);
		UBlueprintFactory* factory = NewObject<UBlueprintFactory>();
		IAssetTools& assetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		bpActor = static_cast<UBlueprint*>(assetTools.CreateAsset(bpName, "/Game/ValorantContent/Blueprints/", UBlueprint::StaticClass(), factory));
		
		if (newJsons.IsEmpty())
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: No nodes in BP %s, skipping"), *bpName);
			continue;
		}
		// Get all characters after '.' using GetExtension. Little hacky but works
		const FString defaultRoot = FPaths::GetExtension(sceneRoots[0]);
		int defaultIndex = newJsons.IndexOfByKey(defaultRoot);
		TSharedPtr<FJsonValue> temp = newJsons[defaultIndex];
		newJsons.RemoveAt(defaultIndex, 1, false);
		newJsons.Push(temp);
		TArray<USCS_Node*> bpNodeArray;
		for (int i = newJsons.Num() - 1; i >= 0; i--)
		{
			const TSharedPtr<FJsonValue> bpNode = newJsons[i];
			const TSharedPtr<FJsonObject> bpNodeObj = bpNode.Get()->AsObject();
			const FString bpNodeName = bpNodeObj->GetStringField("Name");
			TSharedPtr<FJsonObject> bpNodeProps = bpNodeObj->GetObjectField("Properties");
			if (childNodes.Contains(bpNodeName)) continue;
			UE_LOG(LogTemp, Display, TEXT("Uiana: Importing BP Node %s"), *bpNodeObj->GetStringField("Name"));
			const FString componentName = bpNodeProps->GetObjectField("ComponentClass")->GetStringField("ObjectName").Replace(TEXT("Class "), TEXT(""), ESearchCase::CaseSensitive);
			UClass* componentClass = FEditorClassUtils::GetClassFromString(componentName);
			if (componentClass == nullptr)
			{
				UE_LOG(LogTemp, Display, TEXT("Uiana: Node Class %s not Unreal Component, skipping"), *componentName);
				continue;
			}
			if (bpNodeProps->HasField("ChildNodes"))
			{
				UE_LOG(LogTemp, Display, TEXT("Uiana: Importing Child Nodes for BP Node %s"), *bpNodeName);
				bpNodeArray = GetLocalBPChildren(bpNodeProps->GetArrayField("ChildNodes"), newJsons, bpActor);
			}
			const FString componentInternalName = bpNodeProps->GetStringField("InternalVariableName");
			UActorComponent* component = UBPFL::CreateBPComp(bpActor, componentClass, FName(*componentInternalName), bpNodeArray);
			if (bpNodeProps->HasField("CompProps"))
			{
				UE_LOG(LogTemp, Display, TEXT("Uiana: BP Node %s has specified component properties, using them."), *bpNodeName);
				bpNodeProps = bpNodeProps->GetObjectField("CompProps");
			}
			UE_LOG(LogTemp, Display, TEXT("Uiana: Setting BP Node %s properties!"), *bpNodeName);
			SetBPSettings(bpNodeProps, component);
			// TODO: Rework this SetBPSettings() + Set editor property section, corresponds with set_mesh_settings and handle_child_nodes
			FTransform transform = UianaHelpers::GetTransformComponent(bpNodeProps);
			UianaHelpers::SetActorProperty<FVector>(componentClass, component, "RelativeLocation", transform.GetLocation());
			// FObjectEditorUtils::SetPropertyValue(componentNode, FName("RelativeLocation"), transform.GetLocation());
			UianaHelpers::SetActorProperty<FRotator>(componentClass, component, "RelativeRotation", transform.GetRotation().Rotator());
			// FObjectEditorUtils::SetPropertyValue(componentNode, FName("RelativeRotation"), transform.GetRotation());
			UianaHelpers::SetActorProperty<FVector>(componentClass, component, "RelativeScale3D", transform.GetScale3D());
			// FObjectEditorUtils::SetPropertyValue(componentNode, FName("RelativeScale3D"), transform.GetScale3D());
		}
		if (bpName.Equals("SpawnBarrier")) continue;
		for (TSharedPtr<FJsonValue> gameObj : gameObjs)
		{
			UClass* componentClass = UStaticMeshComponent::StaticClass();
			UActorComponent* component = UBPFL::CreateBPComp(bpActor, componentClass, FName("GameObjectMesh"), bpNodeArray);
			const TSharedPtr<FJsonObject> gameObjProps = gameObj->AsObject()->GetObjectField("Properties");
			SetBPSettings(gameObjProps, component);
			FTransform transform = UianaHelpers::GetTransformComponent(gameObjProps);
			// TODO: Make this function with above!
			UianaHelpers::SetActorProperty<FVector>(componentClass, component, "RelativeLocation", transform.GetLocation());
			// FObjectEditorUtils::SetPropertyValue(componentNode, FName("RelativeLocation"), transform.GetLocation());
			// FObjectEditorUtils::SetPropertyValue(componentNode, FName("RelativeRotation"), transform.GetRotation());
			UianaHelpers::SetActorProperty<FVector>(componentClass, component, "RelativeScale3D", transform.GetScale3D());
			// FObjectEditorUtils::SetPropertyValue(componentNode, FName("RelativeScale3D"), transform.GetScale3D());
			UianaHelpers::SetActorProperty<FRotator>(componentClass, component, "RelativeRotation", transform.GetRotation().Rotator());
		}
	}
}

void BlueprintImporter::ImportBlueprint(const TSharedPtr<FJsonObject> obj, TMap<FString, AActor*> &bpMapping)
{
	if (!obj->HasField("Properties")) return;
	FTransform transform;
	if (UianaHelpers::HasTransformComponent(obj->GetObjectField("Properties")))
	{
		transform = UianaHelpers::GetTransformComponent(obj->GetObjectField("Properties"));
	}
	else
	{
		transform = UianaHelpers::GetSceneTransformComponent(obj->GetObjectField("Properties"));
	}
	const FString bpName = obj->GetStringField("Type");//.Mid(0, obj->GetStringField("Type").Len() - 2);
	UClass* bpClass = UEditorAssetLibrary::LoadBlueprintClass(FPaths::Combine("/Game/ValorantContent/Blueprints/", bpName + "." + bpName));
	AActor* bpActor = UEditorLevelLibrary::SpawnActorFromObject(bpClass, transform.GetTranslation(), transform.GetRotation().Rotator());
	if (bpActor == nullptr) return;
	bpActor->SetActorLabel(obj->GetStringField("Name"));
	bpActor->SetActorScale3D(transform.GetScale3D());
	bpMapping.Add(bpName, bpActor);
}

TArray<USCS_Node*> BlueprintImporter::GetLocalBPChildren(TArray<TSharedPtr<FJsonValue>> childNodes, TArray<TSharedPtr<FJsonValue>> bpData, UBlueprint* bpActor)
{
	TArray<USCS_Node*> localChildren;
	for (const TSharedPtr<FJsonValue> childNode : childNodes)
	{
		const TSharedPtr<FJsonObject> childObj = childNode->AsObject();
		const FString childName = childObj->GetStringField("ObjectName");
		for (const TSharedPtr<FJsonValue> cNode : bpData)
		{
			const TSharedPtr<FJsonObject> nodeDataObj = cNode->AsObject();
			// FString temp, componentName;
			// nodeDataObj->GetObjectField("Properties")->GetObjectField("ComponentClass")->GetStringField("ObjectName").Split(TEXT("."), &temp, &componentName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			const FString componentName = nodeDataObj->GetObjectField("Properties")->GetObjectField("ComponentClass")->GetStringField("ObjectName").Replace(TEXT("Class "), TEXT(""), ESearchCase::CaseSensitive);
			UClass* componentClass = FEditorClassUtils::GetClassFromString(componentName);
			if (componentClass == nullptr) continue;
			const FString internalName = nodeDataObj->GetObjectField("Properties")->GetStringField("InternalVariableName");
			if (internalName.Contains("TargetViewMode") || internalName.Contains("Decal1") || internalName.Contains("SM_Barrier_Back_VisionBlocker")) continue;

			if (nodeDataObj->GetStringField("Name").Equals(childName))
			{
				UActorComponent* componentNode;
				USCS_Node* unrealNode = UBPFL::CreateNode(bpActor, componentClass, FName(*internalName), componentNode);
				SetBPSettings(nodeDataObj->GetObjectField("Properties")->GetObjectField("CompProps"), componentNode);
				if (UianaHelpers::HasTransformComponent(nodeDataObj->GetObjectField("Properties")->GetObjectField("CompProps")))
				{
					FTransform transform = UianaHelpers::GetTransformComponent(nodeDataObj->GetObjectField("Properties")->GetObjectField("CompProps"));
					UianaHelpers::SetActorProperty<FTransform>(UActorComponent::StaticClass(), componentNode, "RelativeLocation", transform);
					// FObjectEditorUtils::SetPropertyValue(componentNode, FName("RelativeLocation"), transform.GetLocation());
					UianaHelpers::SetActorProperty<FTransform>(UActorComponent::StaticClass(), componentNode, "RelativeRotation", transform);
					// FObjectEditorUtils::SetPropertyValue(componentNode, FName("RelativeRotation"), transform.GetRotation());
					UianaHelpers::SetActorProperty<FTransform>(UActorComponent::StaticClass(), componentNode, "RelativeScale3D", transform);
					// FObjectEditorUtils::SetPropertyValue(componentNode, FName("RelativeScale3D"), transform.GetScale3D());
				}
				break;
			}
		}
	}
	return localChildren;
}

void BlueprintImporter::SetBPSettings(const TSharedPtr<FJsonObject> bpProps, UActorComponent* bp)
{
	// Loop through all JSON values (type <FString, TSharedPtr<FJsonValue>>)
	for (auto const& prop : bpProps.Get()->Values)
	{
		TSharedPtr<FJsonValue> propValue = prop.Value;
		const FName propName = FName(*prop.Key);
		const FProperty* objectProp = PropertyAccessUtil::FindPropertyByName(propName, bp->GetClass());
		if (objectProp == nullptr) continue;
		const EJson propType = propValue.Get()->Type;
		if (propType == EJson::Number || propType == EJson::Boolean)
		{
			if (prop.Key.Equals("InfluenceRadius") && propValue.Get()->AsNumber() == 0)
			{
				UianaHelpers::SetActorProperty<float>(bp->GetClass(), bp, prop.Key, 14680.0);
				// FObjectEditorUtils::SetPropertyValue(bp, propName, 14680);
				continue;
			}
			UianaHelpers::SetActorProperty<float>(bp->GetClass(), bp, prop.Key, prop.Value.Get()->AsNumber());
			// FObjectEditorUtils::SetPropertyValue(bp, propName, prop.Value.Get()->AsNumber());
			continue;
		}
		if (objectProp->GetClass()->GetName().Equals("FLinearColor"))
		{
			FLinearColor color;
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			color.R = obj->GetNumberField("R");
			color.G = obj->GetNumberField("G");
			color.B = obj->GetNumberField("B");
			UianaHelpers::SetActorProperty<FLinearColor>(bp->GetClass(), bp, prop.Key, color);
			// FObjectEditorUtils::SetPropertyValue(bp, propName, color);
		}
		else if (objectProp->GetClass()->GetName().Equals("FVector4"))
		{
			FVector4 vector;
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			vector.X = obj->GetNumberField("X");
			vector.Y = obj->GetNumberField("Y");
			vector.Z = obj->GetNumberField("Z");
			vector.W = obj->GetNumberField("W");
			UianaHelpers::SetActorProperty<FVector4>(bp->GetClass(), bp, prop.Key, vector);
			// FObjectEditorUtils::SetPropertyValue(bp, propName, vector);
		}
		else if (objectProp->GetClass()->GetName().Contains("Color"))
		{
			FColor color;
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			color.R = obj->GetNumberField("R");
			color.G = obj->GetNumberField("G");
			color.B = obj->GetNumberField("B");
			color.A = obj->GetNumberField("A");
			UianaHelpers::SetActorProperty<FColor>(bp->GetClass(), bp, prop.Key, color);
			// FObjectEditorUtils::SetPropertyValue(bp, propName, color);
		}
		else if (propType == EJson::Object)
		{
			if (prop.Key.Equals("IESTexture"))
			{
				FString temp, newTextureName;
				propValue.Get()->AsObject()->GetStringField("ObjectName").Split("_", &temp, &newTextureName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
				FString assetPath = "/UianaCPP/IESProfiles/" + newTextureName + "." + newTextureName;
				UTexture* newTexture = static_cast<UTexture*>(UEditorAssetLibrary::LoadAsset(assetPath));
				UianaHelpers::SetActorProperty<UTexture*>(bp->GetClass(), bp, prop.Key, newTexture);
				// FObjectEditorUtils::SetPropertyValue(bp, propName, newTexture);
			}
			else if (prop.Key.Equals("Cubemap"))
			{
				FString newCubemapName = propValue.Get()->AsObject()->GetStringField("ObjectName").Replace(TEXT("TextureCube "), TEXT(""));
				FString assetPath = "/UianaCPP/CubeMaps/" + newCubemapName + "." + newCubemapName;
				// TODO: Convert all static_cast with UObjects to Cast<>()
				UTextureCube* newCube = Cast<UTextureCube, UObject>(UEditorAssetLibrary::LoadAsset(assetPath));
				UianaHelpers::SetActorProperty<UTextureCube*>(bp->GetClass(), bp, prop.Key, newCube);
				// FObjectEditorUtils::SetPropertyValue(bp, propName, newCube);
			}
			else if (prop.Key.Equals("DecalSize"))
			{
				FVector vec;
				const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
				vec.X = obj->GetNumberField("X");
				vec.Y = obj->GetNumberField("Y");
				vec.Z = obj->GetNumberField("Z");
				UianaHelpers::SetActorProperty<FVector>(bp->GetClass(), bp, prop.Key, vec);
				// FObjectEditorUtils::SetPropertyValue(bp, propName, vec);
			}
			else if (prop.Key.Equals("StaticMesh"))
			{
				FString meshName;
				if (propValue->AsObject()->TryGetStringField("ObjectName", meshName))
				{
					FString name = meshName.Replace(TEXT("StaticMesh "), TEXT(""), ESearchCase::CaseSensitive);
					UStaticMesh* mesh = static_cast<UStaticMesh*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Meshes/" + name));
					if (mesh == nullptr) continue;
					UianaHelpers::SetActorProperty<UStaticMesh*>(bp->GetClass(), bp, prop.Key, mesh);
					// FObjectEditorUtils::SetPropertyValue(bp, propName, mesh);
				}
			}
			else if (prop.Key.Equals("BoxExtent"))
			{
				FVector vec;
				const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
				vec.X = obj->GetNumberField("X");
				vec.Y = obj->GetNumberField("Y");
				vec.Z = obj->GetNumberField("Z");
				UianaHelpers::SetActorProperty<FVector>(bp->GetClass(), bp, "BoxExtent", vec);
				// FObjectEditorUtils::SetPropertyValue(bp, "BoxExtent", vec);
			}
			else if (prop.Key.Equals("LightmassSettings"))
			{
				// TODO: Investigate why lightmass settings do not seem to be getting applied although no error!
				FLightmassMaterialInterfaceSettings lightmassSettings;
				FJsonObjectConverter::JsonObjectToUStruct(propValue.Get()->AsObject().ToSharedRef(), &lightmassSettings);
				if (!UianaHelpers::SetActorProperty<FLightmassMaterialInterfaceSettings>(bp->GetClass(), bp, "LightmassSettings", lightmassSettings)) UE_LOG(LogTemp, Error, TEXT("Uiana: Failed to set lightmass settings for BP!"));
				// if (!FObjectEditorUtils::SetPropertyValue(bp, "LightmassSettings", lightmassSettings)) UE_LOG(LogTemp, Error, TEXT("Uiana: Failed to set lightmass settings!"));
			}
		}
		else if (propType == EJson::Array && prop.Key.Equals("OverrideMaterials"))
		{
			TArray<UMaterialInstanceConstant*> overrideMats = {};
			for (TSharedPtr<FJsonValue, ESPMode::ThreadSafe> ovrMat : propValue.Get()->AsArray())
			{
				if (!ovrMat.IsValid() || ovrMat.Get()->IsNull())
				{
					overrideMats.Add(nullptr);
				 	continue;
				}
				if (ovrMat.Get()->Type != EJson::Object)
				{
					FString OutputString;
					TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
					TArray<TSharedPtr<FJsonValue>> TempArr = {ovrMat};
					FJsonSerializer::Serialize(TempArr, Writer);
					UE_LOG(LogTemp, Error, TEXT("Uiana: BP Override Material in Array is not an object and instead is:\n %s!"), *OutputString);
					overrideMats.Add(nullptr);
					continue;
				}
				const TSharedPtr<FJsonObject> matData = ovrMat.Get()->AsObject();
				FString temp, objName;
				matData->GetStringField("ObjectName").Split(" ", &temp, &objName, ESearchCase::Type::IgnoreCase, ESearchDir::FromEnd);
				if (matData->HasTypedField<EJson::Object>("ObjectName")) UE_LOG(LogTemp, Error, TEXT("Uiana: BP Override Material ObjectName is not a string!"));
				if (objName.Contains("MaterialInstanceDynamic")) continue;
				UMaterialInstanceConstant* loaded = nullptr;
				loaded = Cast<UMaterialInstanceConstant>(UEditorAssetLibrary::LoadAsset(FPaths::Combine("/UianaCPP/Materials/", objName)));
				if (loaded == nullptr) loaded = Cast<UMaterialInstanceConstant>(UEditorAssetLibrary::LoadAsset(FPaths::Combine("/Game/ValorantContent/Materials/", objName)));
				if (loaded == nullptr)
				{
					UE_LOG(LogTemp, Display, TEXT("Failed to load override material %s, skipping"), *objName);
					continue;
				}
				overrideMats.Add(loaded);
			}
			UianaHelpers::SetActorProperty<TArray<UMaterialInstanceConstant*>>(bp->GetClass(), bp, prop.Key, overrideMats);
			// FObjectEditorUtils::SetPropertyValue(bp, propName, overrideMats);
		}
		else if (propType == EJson::String)
		{
			if (prop.Key.Equals("Mobility"))
			{
				UianaHelpers::SetActorProperty(bp->GetClass(), bp, prop.Key, UianaHelpers::ParseMobility(propValue->AsString()));
			}
			else if (prop.Key.Equals("DetailMode"))
			{
				UianaHelpers::SetActorProperty(bp->GetClass(), bp, prop.Key, UianaHelpers::ParseDetailMode(propValue->AsString()));
			}
		}
		else
		{
			if (propType == EJson::Array)
			{
				const TArray<TSharedPtr<FJsonValue>> parameterArray = propValue.Get()->AsArray();
				if (prop.Key.Equals("TextureStreamingData"))
				{
					TArray<FMaterialTextureInfo> textures;
					for (const TSharedPtr<FJsonValue> texture : parameterArray)
					{
						FMaterialTextureInfo textureInfo;
						const TSharedPtr<FJsonObject> textureObj = texture.Get()->AsObject();
						FJsonObjectConverter::JsonObjectToUStruct(textureObj.ToSharedRef(), &textureInfo);
						textures.Add(textureInfo);
					}
					UE_LOG(LogTemp, Warning, TEXT("Uiana: Need to set TextureStreamingData for BP!"));
					// bp->SetTextureStreamingData(textures);
				}
				else
				{
					if (prop.Key.Equals("ScalarParameterValues") || prop.Key.Equals("VectorParameterValues") || prop.Key.Equals("TextureParameterValues")) continue;
					for (const TSharedPtr<FJsonValue> parameter : parameterArray)
					{
						const TSharedPtr<FJsonObject> paramObj = parameter.Get()->AsObject();
						// const TSharedPtr<FJsonObject> paramInfo = paramObj.Get()->GetObjectField("ParameterInfo");
						// FMaterialParameterInfo info;
						// FJsonObjectConverter::JsonObjectToUStruct(paramInfo.ToSharedRef(), &info);
						// if (prop.Key.Equals("ScalarParameterValues"))
						// {
						// 	double paramValue = paramObj.Get()->GetNumberField("ParameterValue");
						// 	mat->SetScalarParameterValueEditorOnly(info, paramValue);
						// }
						// else if (prop.Key.Equals("VectorParameterValues"))
						// {
						// 	FLinearColor paramValue;
						// 	const TSharedPtr<FJsonObject> vectorParams = paramObj.Get()->GetObjectField("ParameterValue");
						// 	paramValue.R = vectorParams.Get()->GetNumberField("R");
						// 	paramValue.G = vectorParams.Get()->GetNumberField("G");
						// 	paramValue.B = vectorParams.Get()->GetNumberField("B");
						// 	paramValue.A = vectorParams.Get()->GetNumberField("A");
						// 	mat->SetVectorParameterValueEditorOnly(info, paramValue);
						// }
						// else if (prop.Key.Equals("TextureParameterValues"))
						// {
						// 	FString temp, textureName;
						// 	paramObj.Get()->GetObjectField("ParameterValue").Get()->GetStringField("ObjectName").Split(TEXT(" "), &temp, &textureName);
						// 	UTexture* paramValue = static_cast<UTexture*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Textures/" + textureName));
						// 	mat->SetTextureParameterValueEditorOnly(info, paramValue);
						// }
						// else
						// {
						FString OutputString;
						TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
						FJsonSerializer::Serialize(paramObj.ToSharedRef(), Writer);
						UE_LOG(LogTemp, Warning, TEXT("Uiana: Array BP Property %s unaccounted for"), *prop.Key);
						// }
					}	
				}
			}
			else
			{
				FString type = "None";
				if (propType == EJson::Array) type = "Array";
				if (propType == EJson::Object) type = "Object";
				if (propType == EJson::Boolean) type = "Bool";
				if (propType == EJson::String) type = "String";
				if (propType == EJson::Number) type = "Number";
				if (propType == EJson::Null) type = "Null";
				UE_LOG(LogTemp, Warning, TEXT("Uiana: Need to set BP Property %s of type %s!"), *prop.Key, *type);
				if (propType == EJson::String)
				{
					UE_LOG(LogTemp, Warning, TEXT("Uiana: String BP Property unset - %s: %s"), *prop.Key, *propValue->AsString());
				}
			}
		}
	}
}

void BlueprintImporter::FixActorBP(const TSharedPtr<FJsonObject> bpData, const TMap<FString, AActor*> bpMapping, bool bImportMaterials)
{
	const FName bpComponentName = FName(*bpData->GetStringField("Name"));
	if (!bpMapping.Contains(bpData->GetStringField("Outer"))) return;
	AActor* bpActor = bpMapping[bpData->GetStringField("Outer")];
	UActorComponent* bpComponent = UBPFL::GetComponentByName(bpActor, bpComponentName);
	if (bpComponent == nullptr) return;
	if (bpData->HasField("StaticMesh"))
	{
		const TSharedPtr<FJsonObject> bpProps = bpData->GetObjectField("Properties");
		FString meshName;
		if (bpProps->TryGetStringField("ObjectName", meshName))
		{
			FString name = meshName.Replace(TEXT("StaticMesh "), TEXT(""), ESearchCase::CaseSensitive);
			UStaticMesh* mesh = static_cast<UStaticMesh*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Meshes/" + name));
			UianaHelpers::SetActorProperty<UStaticMesh*>(UStaticMesh::StaticClass(), bpComponent, "StaticMesh", mesh);
		}
		if (bImportMaterials && bpProps->HasField("OverrideMaterials"))
		{
			TArray<UMaterialInterface*> overrideMats = MaterialImporter::CreateOverrideMaterials(bpData);
			if (!overrideMats.IsEmpty() && !bpData->GetStringField("Name").Contains("Barrier"))
			{
				UBPFL::SetOverrideMaterial(bpActor, bpComponentName, overrideMats);
			}
		}
		if (bpProps->HasField("AttachParent") && UianaHelpers::HasTransformComponent(bpData->GetObjectField("Properties")))
		{
			FTransform transform = UianaHelpers::GetTransformComponent(bpProps);
			if (bpProps->HasField("RelativeScale3D")) UianaHelpers::SetActorProperty(UStaticMesh::StaticClass(), bpComponent, "RelativeScale3D", transform.GetScale3D());
			if (bpProps->HasField("RelativeLocation")) UianaHelpers::SetActorProperty(UStaticMesh::StaticClass(), bpComponent, "RelativeLocation", transform.GetTranslation());
			if (bpProps->HasField("RelativeRotation")) UianaHelpers::SetActorProperty(UStaticMesh::StaticClass(), bpComponent, "RelativeRotation", transform.GetRotation().Rotator());
		}
	}
}