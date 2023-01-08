#include "MeshBlueprintImporter.h"

MeshBlueprintImporter::MeshBlueprintImporter() : TBaseImporter<UActorComponent>()
{
}

MeshBlueprintImporter::MeshBlueprintImporter(const UianaSettings* UianaSettings) : TBaseImporter<UActorComponent>(UianaSettings)
{
}

void MeshBlueprintImporter::CreateBlueprints(const TArray<FString> BPPaths)
{
	TArray<FString> SceneRoots, ChildNodes;
	TArray<TSharedPtr<FJsonValue>> NewJsons, GameObjs;
	for (const FString BPPath : BPPaths)
	{
		UE_LOG(LogTemp, Display, TEXT("Uiana: Importing BP at path: %s"), *BPPath);
		const TArray<FString> BlacklistedBPs = {"SoundBarrier", "SpawnBarrierProjectile", "BP_UnwalkableBlockingVolumeCylinder",
				   "BP_StuckPickupVolume", "BP_BlockingVolume", "TargetingBlockingVolume_Box", "directional_look_up" };
		const FString bpName = FPaths::GetBaseFilename(BPPath);
		if (BlacklistedBPs.Contains(bpName)) continue;
		
		// Reduce BP JSON
		FString bpJsonStr;
		FFileHelper::LoadFileToString(bpJsonStr, *BPPath);
		TArray<TSharedPtr<FJsonValue>> BPRaws, BPFiltereds;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(bpJsonStr);
		if (!FJsonSerializer::Deserialize(JsonReader, BPRaws) || !BPRaws[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize blueprint %s"), *BPPath);
			continue;
		}
		for (const TSharedPtr<FJsonValue> BP : BPRaws)
		{
			const TSharedPtr<FJsonObject> BPObj = BP->AsObject();
			if (!BPObj->HasField("Properties")) continue;
			const TSharedPtr<FJsonObject> bpProps = BPObj->GetObjectField("Properties");
			const FString Name = BPObj->GetStringField("Name");
			const FString Type = BPObj->GetStringField("Type");
			if (Type.Equals("SimpleConstructionScript"))
			{
				const FString sceneRoot = bpProps->GetObjectField("DefaultSceneRootNode")->GetStringField("ObjectName");
				FString temp, root;
				sceneRoot.Split(TEXT(":"), &temp, &root, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
				SceneRoots.Add(root);
			}
			if (Name.Contains("Node"))
			{
				const FString objName = bpProps->GetObjectField("ComponentTemplate")->GetStringField("ObjectName");
				FString temp, nodeName;
				objName.Split(TEXT(":"), &temp, &nodeName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
				TSharedPtr<FJsonObject> nodeComponent;
				for (const TSharedPtr<FJsonValue> componentBP : BPRaws)
				{
					if (componentBP->AsObject()->HasField("Name"))
					{
						nodeComponent = componentBP->AsObject();
						break;
					}
				}
				if (nodeComponent.IsValid() && nodeComponent->HasField("Properties")) // TODO: CAUSE FOR CRASH - nodeComponent is null and causes crash
					bpProps->SetObjectField("CompProps", nodeComponent->GetObjectField("Properties"));
				NewJsons.Add(BP);
				if (bpProps->HasField("ChildNodes"))
				{
					for (const TSharedPtr<FJsonValue> childNode : bpProps->GetArrayField("ChildNodes"))
					{
						FString temp1, childName;
						const FString childNameRaw = childNode.Get()->AsObject()->GetStringField("ObjectName");
						childNameRaw.Split(TEXT("."), &temp1, &childName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
						ChildNodes.Add(childName);
					}
				}
			}
			if (Name.Equals("GameObjectMesh"))
			{
				GameObjs.Add(BP);
			}
		}
		// FJsonObject bpJson = FJsonObject();
		// bpJson.SetArrayField("Nodes", newJsons);
		// bpJson.SetArrayField("SceneRoot", sceneRoots);
		// bpJson.SetArrayField("GameObjects", gameObjs);
		// bpJson.SetArrayField("ChildNodes", childNodes);

		// Create BP
		UE_LOG(LogTemp, Display, TEXT("Uiana: Checking if BP %s exists"), *bpName);
		UBlueprint* BPActor = Cast<UBlueprint>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Blueprints/" + bpName));
		if (BPActor != nullptr) continue;
		UE_LOG(LogTemp, Display, TEXT("Uiana: Creating BP %s"), *bpName);
		UBlueprintFactory* BPFactory = NewObject<UBlueprintFactory>();
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		BPActor = static_cast<UBlueprint*>(AssetTools.CreateAsset(bpName, "/Game/ValorantContent/Blueprints/", UBlueprint::StaticClass(), BPFactory));
		
		if (NewJsons.IsEmpty())
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: No nodes in BP %s, skipping"), *bpName);
			continue;
		}
		// Get all characters after '.' using GetExtension. Little hacky but works
		const FString DefaultRoot = FPaths::GetExtension(SceneRoots[0]);
		int DefaultIndex = NewJsons.IndexOfByKey(DefaultRoot);
		TSharedPtr<FJsonValue> Temp = NewJsons[DefaultIndex];
		NewJsons.RemoveAt(DefaultIndex, 1, false);
		NewJsons.Push(Temp);
		TArray<USCS_Node*> BPNodeArray;
		for (int i = NewJsons.Num() - 1; i >= 0; i--)
		{
			const TSharedPtr<FJsonValue> BPNode = NewJsons[i];
			const TSharedPtr<FJsonObject> BPNodeObj = BPNode.Get()->AsObject();
			const FString BPNodeName = BPNodeObj->GetStringField("Name");
			TSharedPtr<FJsonObject> bpNodeProps = BPNodeObj->GetObjectField("Properties");
			if (ChildNodes.Contains(BPNodeName)) continue;
			UE_LOG(LogTemp, Display, TEXT("Uiana: Importing BP Node %s"), *BPNodeObj->GetStringField("Name"));
			const FString ComponentName = bpNodeProps->GetObjectField("ComponentClass")->GetStringField("ObjectName").Replace(TEXT("Class "), TEXT(""), ESearchCase::CaseSensitive);
			UClass* ComponentClass = FEditorClassUtils::GetClassFromString(ComponentName);
			if (ComponentClass == nullptr)
			{
				continue;
			}
			if (bpNodeProps->HasField("ChildNodes"))
			{
				BPNodeArray = GetLocalBPChildren(bpNodeProps->GetArrayField("ChildNodes"), NewJsons, BPActor);
			}
			const FString componentInternalName = bpNodeProps->GetStringField("InternalVariableName");
			UActorComponent* component = UBPFL::CreateBPComp(BPActor, ComponentClass, FName(*componentInternalName), BPNodeArray);
			if (bpNodeProps->HasField("CompProps"))
			{
				bpNodeProps = bpNodeProps->GetObjectField("CompProps");
			}
			SetSettingsFromJsonProperties(bpNodeProps, component);
		}
		if (bpName.Equals("SpawnBarrier")) continue;
		for (TSharedPtr<FJsonValue> GameObj : GameObjs)
		{
			UClass* ComponentClass = UStaticMeshComponent::StaticClass();
			UActorComponent* Component = UBPFL::CreateBPComp(BPActor, ComponentClass, FName("GameObjectMesh"), BPNodeArray);
			const TSharedPtr<FJsonObject> GameObjProps = GameObj->AsObject()->GetObjectField("Properties");
			SetSettingsFromJsonProperties(GameObjProps, Component);
		}
	}
}

void MeshBlueprintImporter::ImportBlueprint(const TSharedPtr<FJsonObject> Obj, TMap<FString, AActor*> &BPMapping)
{
	if (!Obj->HasField("Properties")) return;
	FTransform Transform;
	if (UianaHelpers::HasTransformComponent(Obj->GetObjectField("Properties")))
	{
		Transform = UianaHelpers::GetTransformComponent(Obj->GetObjectField("Properties"));
	}
	else
	{
		Transform = UianaHelpers::GetSceneTransformComponent(Obj->GetObjectField("Properties"));
	}
	const FString BPName = Obj->GetStringField("Type");
	UClass* BPClass = UEditorAssetLibrary::LoadBlueprintClass(FPaths::Combine("/Game/ValorantContent/Blueprints/", BPName + "." + BPName));
	AActor* BPActor = UEditorLevelLibrary::SpawnActorFromObject(BPClass, Transform.GetTranslation(), Transform.GetRotation().Rotator());
	if (BPActor == nullptr) return;
	BPActor->SetActorLabel(Obj->GetStringField("Name"));
	BPActor->SetActorScale3D(Transform.GetScale3D());
	BPMapping.Add(BPName, BPActor);
}

TArray<USCS_Node*> MeshBlueprintImporter::GetLocalBPChildren(TArray<TSharedPtr<FJsonValue>> ChildNodes, TArray<TSharedPtr<FJsonValue>> BPData, UBlueprint* BPActor)
{
	TArray<USCS_Node*> LocalChildren;
	for (const TSharedPtr<FJsonValue> ChildNode : ChildNodes)
	{
		const TSharedPtr<FJsonObject> ChildObj = ChildNode->AsObject();
		const FString ChildName = ChildObj->GetStringField("ObjectName");
		for (const TSharedPtr<FJsonValue> BPChildNode : BPData)
		{
			const TSharedPtr<FJsonObject> NodeDataObj = BPChildNode->AsObject();
			const FString ComponentName = NodeDataObj->GetObjectField("Properties")->GetObjectField("ComponentClass")->GetStringField("ObjectName").Replace(TEXT("Class "), TEXT(""), ESearchCase::CaseSensitive);
			UClass* ComponentClass = FEditorClassUtils::GetClassFromString(ComponentName);
			if (ComponentClass == nullptr) continue;
			const FString InternalName = NodeDataObj->GetObjectField("Properties")->GetStringField("InternalVariableName");
			if (InternalName.Contains("TargetViewMode") || InternalName.Contains("Decal1") || InternalName.Contains("SM_Barrier_Back_VisionBlocker")) continue;

			if (NodeDataObj->GetStringField("Name").Equals(ChildName))
			{
				UActorComponent* ComponentNode;
				USCS_Node* UnrealNode = UBPFL::CreateNode(BPActor, ComponentClass, FName(*InternalName), ComponentNode);
				LocalChildren.Add(UnrealNode);
				SetSettingsFromJsonProperties(NodeDataObj->GetObjectField("Properties")->GetObjectField("CompProps"), ComponentNode);
				break;
			}
		}
	}
	return LocalChildren;
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
	const bool bIsInstanced = Obj->HasField("PerInstanceSMData") && Obj->GetStringField("Type").Contains("Instanced");
	UClass* MeshActorClass = bIsInstanced ? AHismActorCPP::StaticClass() : AStaticMeshActor::StaticClass();
	AActor* MeshActor = UEditorLevelLibrary::SpawnActorFromClass(MeshActorClass, FVector::ZeroVector);
	MeshActor->SetActorLabel(Obj->GetStringField("Outer"));
	TArray<UObject*> MeshActorObjects;
	MeshActor->GetDefaultSubobjects(MeshActorObjects);
	UHierarchicalInstancedStaticMeshComponent* MeshInstancedObject = Cast<UHierarchicalInstancedStaticMeshComponent>(MeshActorObjects.Last());
	UStaticMeshComponent* MeshObject;
	if (bIsInstanced)
	{
		MeshActor->SetFolderPath("Meshes/Instanced");
		const TArray<TSharedPtr<FJsonValue>> PerInstanceData = Obj->GetArrayField("PerInstanceSMData");
		for (const TSharedPtr<FJsonValue> Instance : PerInstanceData)
		{
			// Do not need to access Properties attribute for instance data
			FTransform InstanceTransform = UianaHelpers::GetTransformComponent(Instance->AsObject());
			MeshInstancedObject->AddInstance(InstanceTransform);
		}
		MeshObject = MeshInstancedObject;
	}
	else
	{
		FString UmapType = UmapName.Contains("_VFX") ? "VFX" : "Static";
		MeshActor->SetFolderPath(FName(*FPaths::Combine("Meshes", UmapType)));
		MeshObject = Cast<UStaticMeshComponent>(MeshActorObjects.Last());
	}
	SetSettingsFromJsonProperties(Obj->GetObjectField("Properties"), MeshObject); // TODO: If there are no bugs with this, rename to "SetActorSettings()"!
	MeshActor->PostEditMove(true);
	if (Obj->HasField("LODData"))
	{
		const TArray<TSharedPtr<FJsonValue>> LODData = Obj->GetArrayField("LODData");
		TArray<FColor> VtxArray = {};
		for (const TSharedPtr<FJsonValue> LOD : LODData)
		{
			if (const TSharedPtr<FJsonObject> LODObj = LOD->AsObject(); LODObj->HasField("OverrideVertexColors"))
			{
				const TArray<TSharedPtr<FJsonValue>> VertexData = LODObj->GetObjectField("OverrideVertexColors")->GetArrayField("Data");
				for (const TSharedPtr<FJsonValue> Color : VertexData)
				{
					VtxArray.Add(UBPFL::ReturnFromHex(Color->AsString()));
				}
			}
		}
		FString ModelPath = "NoPath";
		if (Obj->GetObjectField("Properties")->HasField("StaticMesh"))
		{
			ModelPath = FPaths::Combine(
					Settings->ExportAssetsPath.Path,
					FPaths::GetBaseFilename(
						Obj->GetObjectField("Properties")->GetObjectField("StaticMesh")->GetStringField("ObjectPath"),
						false
						)
					)
					.Replace(TEXT("ShooterGame"), TEXT("Game"), ESearchCase::CaseSensitive)
					.Replace(TEXT("/Content"), TEXT(""), ESearchCase::CaseSensitive) + ".pskx";
		}
		if (!VtxArray.IsEmpty())
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: Painting %d SM Vertices for mesh %s with modelPath %s"), VtxArray.Num(), *MeshActor->GetActorLabel(), *ModelPath);
			UBPFL::PaintSMVertices(MeshObject, VtxArray, ModelPath);
		}
	}
	if (Settings->ImportMaterials && Obj->GetObjectField("Properties")->HasField("OverrideMaterials"))
	{
		if (TArray<UMaterialInterface*> OverrideMats = CreateOverrideMaterials(Obj); !OverrideMats.IsEmpty())
		{
			UianaHelpers::SetActorProperty(UStaticMeshComponent::StaticClass(), MeshObject, "OverrideMaterials", OverrideMats);	
		}
	}
}

void MeshBlueprintImporter::FixActorBP(const TSharedPtr<FJsonObject> BPData, const TMap<FString, AActor*> BPMapping, const bool bImportMaterials) const
{
	const FName bpComponentName = FName(*BPData->GetStringField("Name"));
	if (!BPMapping.Contains(BPData->GetStringField("Outer"))) return;
	AActor* BPActor = BPMapping[BPData->GetStringField("Outer")];
	UActorComponent* BPComponent = UBPFL::GetComponentByName(BPActor, bpComponentName);
	if (BPComponent == nullptr) return;
	const TSharedPtr<FJsonObject> BPProps = BPData->GetObjectField("Properties");
	if (BPProps->HasField("StaticMesh"))
	{
		if (FString MeshName; BPProps->TryGetStringField("ObjectName", MeshName))
		{
			const FString Name = MeshName.Replace(TEXT("StaticMesh "), TEXT(""), ESearchCase::CaseSensitive);
			UStaticMesh* Mesh = static_cast<UStaticMesh*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Meshes/" + Name));
			UianaHelpers::SetActorProperty<UStaticMesh*>(UStaticMesh::StaticClass(), BPComponent, "StaticMesh", Mesh);
		}
	}
	if (bImportMaterials && BPProps->HasField("OverrideMaterials"))
	{
		const TArray<UMaterialInterface*> OverrideMats = CreateOverrideMaterials(BPData);
		if (!OverrideMats.IsEmpty() && !BPData->GetStringField("Name").Contains("Barrier"))
		{
			UBPFL::SetOverrideMaterial(BPActor, bpComponentName, OverrideMats);
		}
	}
	if (BPProps->HasField("AttachParent") && UianaHelpers::HasTransformComponent(BPProps))
	{
		const FTransform Transform = UianaHelpers::GetTransformComponent(BPProps);
		if (BPProps->HasField("RelativeScale3D")) UianaHelpers::SetActorProperty(UStaticMesh::StaticClass(), BPComponent, "RelativeScale3D", Transform.GetScale3D());
		if (BPProps->HasField("RelativeLocation")) UianaHelpers::SetActorProperty(UStaticMesh::StaticClass(), BPComponent, "RelativeLocation", Transform.GetTranslation());
		if (BPProps->HasField("RelativeRotation")) UianaHelpers::SetActorProperty(UStaticMesh::StaticClass(), BPComponent, "RelativeRotation", Transform.GetRotation().Rotator());
	}
}

TArray<UMaterialInterface*> MeshBlueprintImporter::CreateOverrideMaterials(const TSharedPtr<FJsonObject> Obj) const
{
	TArray<UMaterialInterface*> mats = {};
	for (const TSharedPtr<FJsonValue> mat : Obj->GetObjectField("Properties")->GetArrayField("OverrideMaterials"))
	{
		if (!mat.IsValid() || mat->IsNull())
		{
			mats.Add(nullptr);
			continue;
		}
		FString ObjName, Temp;
		mat->AsObject()->GetStringField("ObjectName").Split(TEXT(" "), &Temp, &ObjName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		if (ObjName.Equals("Stone_M2_Steps_MI1")) ObjName = "Stone_M2_Steps_MI";
		if (ObjName.Contains("MaterialInstanceDynamic"))
		{
			mats.Add(nullptr);
			continue;
		}
		mats.Add(Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(FPaths::Combine("/Game/ValorantContent/Materials/", ObjName))));
	}
	return mats;
}

bool MeshBlueprintImporter::OverrideObjectProp(const FString JsonPropName, const TSharedPtr<FJsonValue> JsonPropValue,
	const FProperty* ObjectProp, UActorComponent* BaseObj)
{
	if (JsonPropName.Equals("StaticMesh"))
	{
		// Not Lights, Decals | Meshes, Blueprints
		FString MeshName;
		if (JsonPropValue->AsObject()->TryGetStringField("ObjectName", MeshName))
		{
			const FString Name = MeshName.Replace(TEXT("StaticMesh "), TEXT(""), ESearchCase::CaseSensitive);
			UStaticMesh* Mesh = static_cast<UStaticMesh*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Meshes/" + Name));
			if (Mesh == nullptr) return false;
			UianaHelpers::SetActorProperty<UStaticMesh*>(BaseObj->GetClass(), BaseObj, JsonPropName, Mesh);
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
		TArray<UMaterialInstanceConstant*> OverrideMats = {};
		for (TSharedPtr<FJsonValue, ESPMode::ThreadSafe> OverrideMatJson : JsonPropValue.Get()->AsArray())
		{
			if (!OverrideMatJson.IsValid() || OverrideMatJson.Get()->IsNull())
			{
				OverrideMats.Add(nullptr);
				continue;
			}
			const TSharedPtr<FJsonObject> matData = OverrideMatJson.Get()->AsObject();
			FString Temp, ObjName;
			matData->GetStringField("ObjectName").Split(" ", &Temp, &ObjName, ESearchCase::Type::IgnoreCase, ESearchDir::FromEnd);
			if (matData->HasTypedField<EJson::Object>("ObjectName")) UE_LOG(LogTemp, Error, TEXT("Uiana: BP Override Material ObjectName is not a string!"));
			if (ObjName.Contains("MaterialInstanceDynamic"))
			{
				OverrideMats.Add(nullptr);
				continue;
			}
			UMaterialInstanceConstant* Loaded = nullptr;
			Loaded = Cast<UMaterialInstanceConstant>(UEditorAssetLibrary::LoadAsset(FPaths::Combine("/UianaCPP/Materials/", ObjName)));
			if (Loaded == nullptr) Loaded = Cast<UMaterialInstanceConstant>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Materials/" + ObjName));
			OverrideMats.Add(Loaded);
		}
		UianaHelpers::SetActorProperty<TArray<UMaterialInstanceConstant*>>(BaseObj->GetClass(), BaseObj, JsonPropName, OverrideMats);
	}
	else if (JsonPropName.Equals("StreamingTextureData"))
	{
		// Skip this attribute, I do not know how to handle this one
	}
	else
	{
		return false;
	}
	return true;
}
