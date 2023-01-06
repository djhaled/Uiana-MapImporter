#include "MeshBlueprintImporter.h"

MeshBlueprintImporter::MeshBlueprintImporter() : TBaseImporter<UActorComponent>()
{
}

MeshBlueprintImporter::MeshBlueprintImporter(const UianaSettings* UianaSettings) : TBaseImporter<UActorComponent>(UianaSettings)
{
}

void MeshBlueprintImporter::CreateBlueprints(const TArray<FString> BPPaths)
{
	TArray<FString> sceneRoots, childNodes;
	TArray<TSharedPtr<FJsonValue>> newJsons, gameObjs;
	for (const FString bpPath : BPPaths)
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
				continue;
			}
			if (bpNodeProps->HasField("ChildNodes"))
			{
				bpNodeArray = GetLocalBPChildren(bpNodeProps->GetArrayField("ChildNodes"), newJsons, bpActor);
			}
			const FString componentInternalName = bpNodeProps->GetStringField("InternalVariableName");
			UActorComponent* component = UBPFL::CreateBPComp(bpActor, componentClass, FName(*componentInternalName), bpNodeArray);
			if (bpNodeProps->HasField("CompProps"))
			{
				bpNodeProps = bpNodeProps->GetObjectField("CompProps");
			}
			SetSettingsFromJsonProperties(bpNodeProps, component);
		}
		if (bpName.Equals("SpawnBarrier")) continue;
		for (TSharedPtr<FJsonValue> gameObj : gameObjs)
		{
			UClass* componentClass = UStaticMeshComponent::StaticClass();
			UActorComponent* component = UBPFL::CreateBPComp(bpActor, componentClass, FName("GameObjectMesh"), bpNodeArray);
			const TSharedPtr<FJsonObject> gameObjProps = gameObj->AsObject()->GetObjectField("Properties");
			SetSettingsFromJsonProperties(gameObjProps, component);
		}
	}
}

void MeshBlueprintImporter::ImportBlueprint(const TSharedPtr<FJsonObject> Obj, TMap<FString, AActor*> &BPMapping)
{
	if (!Obj->HasField("Properties")) return;
	FTransform transform;
	if (UianaHelpers::HasTransformComponent(Obj->GetObjectField("Properties")))
	{
		transform = UianaHelpers::GetTransformComponent(Obj->GetObjectField("Properties"));
	}
	else
	{
		transform = UianaHelpers::GetSceneTransformComponent(Obj->GetObjectField("Properties"));
	}
	const FString bpName = Obj->GetStringField("Type");
	UClass* bpClass = UEditorAssetLibrary::LoadBlueprintClass(FPaths::Combine("/Game/ValorantContent/Blueprints/", bpName + "." + bpName));
	AActor* bpActor = UEditorLevelLibrary::SpawnActorFromObject(bpClass, transform.GetTranslation(), transform.GetRotation().Rotator());
	if (bpActor == nullptr) return;
	bpActor->SetActorLabel(Obj->GetStringField("Name"));
	bpActor->SetActorScale3D(transform.GetScale3D());
	BPMapping.Add(bpName, bpActor);
}

TArray<USCS_Node*> MeshBlueprintImporter::GetLocalBPChildren(TArray<TSharedPtr<FJsonValue>> ChildNodes, TArray<TSharedPtr<FJsonValue>> BPData, UBlueprint* BPActor)
{
	TArray<USCS_Node*> localChildren;
	for (const TSharedPtr<FJsonValue> childNode : ChildNodes)
	{
		const TSharedPtr<FJsonObject> childObj = childNode->AsObject();
		const FString childName = childObj->GetStringField("ObjectName");
		for (const TSharedPtr<FJsonValue> cNode : BPData)
		{
			const TSharedPtr<FJsonObject> nodeDataObj = cNode->AsObject();
			const FString componentName = nodeDataObj->GetObjectField("Properties")->GetObjectField("ComponentClass")->GetStringField("ObjectName").Replace(TEXT("Class "), TEXT(""), ESearchCase::CaseSensitive);
			UClass* componentClass = FEditorClassUtils::GetClassFromString(componentName);
			if (componentClass == nullptr) continue;
			const FString internalName = nodeDataObj->GetObjectField("Properties")->GetStringField("InternalVariableName");
			if (internalName.Contains("TargetViewMode") || internalName.Contains("Decal1") || internalName.Contains("SM_Barrier_Back_VisionBlocker")) continue;

			if (nodeDataObj->GetStringField("Name").Equals(childName))
			{
				UActorComponent* componentNode;
				USCS_Node* unrealNode = UBPFL::CreateNode(BPActor, componentClass, FName(*internalName), componentNode);
				SetSettingsFromJsonProperties(nodeDataObj->GetObjectField("Properties")->GetObjectField("CompProps"), componentNode);
				break;
			}
		}
	}
	return localChildren;
}

void MeshBlueprintImporter::ImportMesh(const TSharedPtr<FJsonObject> Obj, const FString UmapName, const TMap<FString, AActor*> BPMapping)
{
	if (Obj->HasField("Template"))
	{
		FixActorBP(Obj, BPMapping, Settings->ImportMaterials);
		return;
	}
	if (!Obj->GetObjectField("Properties")->HasField("StaticMesh") || !UianaHelpers::HasTransformComponent(Obj->GetObjectField("Properties")))
	{
		return;
	}
	const bool isInstanced = Obj->HasField("PerInstanceSMData") && Obj->GetStringField("Type").Contains("Instanced");
	UClass* meshActorClass = isInstanced ? AHismActorCPP::StaticClass() : AStaticMeshActor::StaticClass();
	AActor* meshActor = UEditorLevelLibrary::SpawnActorFromClass(meshActorClass, FVector::ZeroVector);
	meshActor->SetActorLabel(Obj->GetStringField("Outer"));
	TArray<UObject*> meshActorObjects;
	meshActor->GetDefaultSubobjects(meshActorObjects);
	UHierarchicalInstancedStaticMeshComponent* meshInstancedObject = Cast<UHierarchicalInstancedStaticMeshComponent>(meshActorObjects.Last());
	UStaticMeshComponent* meshObject;
	if (isInstanced)
	{
		meshActor->SetFolderPath("Meshes/Instanced");
		const TArray<TSharedPtr<FJsonValue>> perInstanceData = Obj->GetArrayField("PerInstanceSMData");
		for (const TSharedPtr<FJsonValue> instance : perInstanceData)
		{
			// Do not need to access Properties attribute for instance data
			FTransform instanceTransform = UianaHelpers::GetTransformComponent(instance->AsObject());
			meshInstancedObject->AddInstance(instanceTransform);
		}
		meshObject = meshInstancedObject;
	}
	else
	{
		FString umapType = UmapName.Contains("_VFX") ? "VFX" : "Static";
		meshActor->SetFolderPath(FName(*FPaths::Combine("Meshes", umapType)));
		meshObject = Cast<UStaticMeshComponent>(meshActorObjects.Last());
	}
	SetSettingsFromJsonProperties(Obj->GetObjectField("Properties"), meshObject); // TODO: If there are no bugs with this, rename to "SetActorSettings()"!
	meshActor->PostEditMove(true);
	if (Obj->HasField("LODData"))
	{
		const TArray<TSharedPtr<FJsonValue>> lodData = Obj->GetArrayField("LODData");
		TArray<FColor> vtxArray = {};
		for (const TSharedPtr<FJsonValue> lod : lodData)
		{
			const TSharedPtr<FJsonObject> lodObj = lod->AsObject();
			if (lodObj->HasField("OverrideVertexColors"))
			{
				const TArray<TSharedPtr<FJsonValue>> vertexData = lodObj->GetObjectField("OverrideVertexColors")->GetArrayField("Data");
				for (const TSharedPtr<FJsonValue> color : vertexData)
				{
					vtxArray.Add(UBPFL::ReturnFromHex(color->AsString()));
				}
			}
		}
		FString modelPath = "NoPath";
		if (Obj->GetObjectField("Properties")->HasField("StaticMesh"))
		{
			modelPath = FPaths::Combine(
					Settings->ExportAssetsPath.Path,
					FPaths::GetBaseFilename(
						Obj->GetObjectField("Properties")->GetObjectField("StaticMesh")->GetStringField("ObjectPath"),
						false
						)
					)
					.Replace(TEXT("ShooterGame"), TEXT("Game"), ESearchCase::CaseSensitive)
					.Replace(TEXT("/Content"), TEXT(""), ESearchCase::CaseSensitive) + ".pskx";
		}
		if (!vtxArray.IsEmpty())
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: Painting %d SM Vertices for mesh %s with modelPath %s"), vtxArray.Num(), *meshActor->GetActorLabel(), *modelPath);
			UBPFL::PaintSMVertices(meshObject, vtxArray, modelPath);
		}
	}
	if (Settings->ImportMaterials && Obj->GetObjectField("Properties")->HasField("OverrideMaterials"))
	{
		TArray<UMaterialInterface*> OverrideMats = CreateOverrideMaterials(Obj);
		if (!OverrideMats.IsEmpty())
		{
			UianaHelpers::SetActorProperty(UStaticMeshComponent::StaticClass(), meshObject, "OverrideMaterials", OverrideMats);	
		}
	}
}

void MeshBlueprintImporter::FixActorBP(const TSharedPtr<FJsonObject> BPData, const TMap<FString, AActor*> BPMapping, bool bImportMaterials)
{
	const FName bpComponentName = FName(*BPData->GetStringField("Name"));
	if (!BPMapping.Contains(BPData->GetStringField("Outer"))) return;
	AActor* bpActor = BPMapping[BPData->GetStringField("Outer")];
	UActorComponent* bpComponent = UBPFL::GetComponentByName(bpActor, bpComponentName);
	if (bpComponent == nullptr) return;
	const TSharedPtr<FJsonObject> bpProps = BPData->GetObjectField("Properties");
	if (bpProps->HasField("StaticMesh"))
	{
		FString meshName;
		if (bpProps->TryGetStringField("ObjectName", meshName))
		{
			FString name = meshName.Replace(TEXT("StaticMesh "), TEXT(""), ESearchCase::CaseSensitive);
			UStaticMesh* mesh = static_cast<UStaticMesh*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Meshes/" + name));
			UianaHelpers::SetActorProperty<UStaticMesh*>(UStaticMesh::StaticClass(), bpComponent, "StaticMesh", mesh);
		}
	}
	if (bImportMaterials && bpProps->HasField("OverrideMaterials"))
	{
		TArray<UMaterialInterface*> overrideMats = CreateOverrideMaterials(BPData);
		if (!overrideMats.IsEmpty() && !BPData->GetStringField("Name").Contains("Barrier"))
		{
			UBPFL::SetOverrideMaterial(bpActor, bpComponentName, overrideMats);
		}
	}
	if (bpProps->HasField("AttachParent") && UianaHelpers::HasTransformComponent(bpProps))
	{
		FTransform transform = UianaHelpers::GetTransformComponent(bpProps);
		if (bpProps->HasField("RelativeScale3D")) UianaHelpers::SetActorProperty(UStaticMesh::StaticClass(), bpComponent, "RelativeScale3D", transform.GetScale3D());
		if (bpProps->HasField("RelativeLocation")) UianaHelpers::SetActorProperty(UStaticMesh::StaticClass(), bpComponent, "RelativeLocation", transform.GetTranslation());
		if (bpProps->HasField("RelativeRotation")) UianaHelpers::SetActorProperty(UStaticMesh::StaticClass(), bpComponent, "RelativeRotation", transform.GetRotation().Rotator());
	}
}

TArray<UMaterialInterface*> MeshBlueprintImporter::CreateOverrideMaterials(const TSharedPtr<FJsonObject> Obj)
{
	TArray<UMaterialInterface*> mats = {};
	for (const TSharedPtr<FJsonValue> mat : Obj->GetObjectField("Properties")->GetArrayField("OverrideMaterials"))
	{
		if (!mat.IsValid() || mat->IsNull())
		{
			mats.Add(nullptr);
			continue;
		}
		FString objName, temp;
		mat->AsObject()->GetStringField("ObjectName").Split(TEXT(" "), &temp, &objName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		if (objName.Equals("Stone_M2_Steps_MI1")) objName = "Stone_M2_Steps_MI";
		if (objName.Contains("MaterialInstanceDynamic"))
		{
			mats.Add(nullptr);
			continue;
		}
		mats.Add(Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(FPaths::Combine("/Game/ValorantContent/Materials/", objName))));
	}
	return mats;
}

bool MeshBlueprintImporter::OverrideObjectProp(const FString JsonPropName, const TSharedPtr<FJsonValue> JsonPropValue,
	const FProperty* ObjectProp, UActorComponent* BaseObj)
{
	if (JsonPropName.Equals("StaticMesh"))
	{
		// Not Lights, Decals | Meshes, Blueprints
		FString meshName;
		if (JsonPropValue->AsObject()->TryGetStringField("ObjectName", meshName))
		{
			FString name = meshName.Replace(TEXT("StaticMesh "), TEXT(""), ESearchCase::CaseSensitive);
			UStaticMesh* mesh = static_cast<UStaticMesh*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Meshes/" + name));
			if (mesh == nullptr) return false;
			UianaHelpers::SetActorProperty<UStaticMesh*>(BaseObj->GetClass(), BaseObj, JsonPropName, mesh);
		}
	}
	else
	{
		return false;
	}
	return true;
}

bool MeshBlueprintImporter::OverrideArrayProp(const FString JsonPropName, const TSharedPtr<FJsonValue> JsonPropValue,
	const FProperty* ObjectProp, UActorComponent* BaseObj)
{
	if (JsonPropName.Equals("OverrideMaterials"))
	{
		// | Meshes, Blueprints
		TArray<UMaterialInstanceConstant*> overrideMats = {};
		for (TSharedPtr<FJsonValue, ESPMode::ThreadSafe> ovrMat : JsonPropValue.Get()->AsArray())
		{
			if (!ovrMat.IsValid() || ovrMat.Get()->IsNull())
			{
				overrideMats.Add(nullptr);
				continue;
			}
			const TSharedPtr<FJsonObject> matData = ovrMat.Get()->AsObject();
			FString temp, objName;
			matData->GetStringField("ObjectName").Split(" ", &temp, &objName, ESearchCase::Type::IgnoreCase, ESearchDir::FromEnd);
			if (matData->HasTypedField<EJson::Object>("ObjectName")) UE_LOG(LogTemp, Error, TEXT("Uiana: BP Override Material ObjectName is not a string!"));
			if (objName.Contains("MaterialInstanceDynamic"))
			{
				overrideMats.Add(nullptr);
				continue;
			}
			UMaterialInstanceConstant* loaded = nullptr;
			loaded = Cast<UMaterialInstanceConstant>(UEditorAssetLibrary::LoadAsset(FPaths::Combine("/UianaCPP/Materials/", objName)));
			if (loaded == nullptr) loaded = Cast<UMaterialInstanceConstant>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Materials/" + objName));
			overrideMats.Add(loaded);
		}
		UianaHelpers::SetActorProperty<TArray<UMaterialInstanceConstant*>>(BaseObj->GetClass(), BaseObj, JsonPropName, overrideMats);
	}
	else
	{
		return false;
	}
	return true;
}
