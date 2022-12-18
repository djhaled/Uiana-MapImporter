#include "UianaImporter.h"
#include "UianaImporter.h"

#include "ActorEditorUtils.h"
#include "AssetImporter.h"
#include "BlueprintImporter.h"
#include "EditorClassUtils.h"
#include "EditorLevelUtils.h"
#include "HismActorCPP.h"
#include "IHeadMountedDisplay.h"
#include "MaterialImporter.h"
#include "Components/BrushComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "Engine/DecalActor.h"
#include "Engine/LevelStreamingAlwaysLoaded.h"
#include "Engine/SCS_Node.h"
#include "Engine/StaticMeshActor.h"
#include "Factories/BlueprintFactory.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/ComponentEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/StructureEditorUtils.h"
#include "Lightmass/LightmassCharacterIndirectDetailVolume.h"

#define LOCTEXT_NAMESPACE "UUianaImporter"

TArray<FString> UUianaImporter::UMaps = {};
UUianaSettings* UUianaImporter::Settings;
AssetImporter UUianaImporter::AssetImporterComp;
MaterialImporter UUianaImporter::MaterialImporterComp;
BlueprintImporter UUianaImporter::BlueprintImporterComp;

UUianaImporter::UUianaImporter()
{
	UE_LOG(LogTemp, Error, TEXT("Uiana: Must initialize importer with parameters!"));
}

void UUianaImporter::Initialize(FString MapName, UUianaCPPDataSettings* InputSettings)
{
	Settings = NewObject<UUianaSettings>();
	Settings->Initialize(MapName, InputSettings);
	FString InputSettingsStr = "INPUT SETTINGS: ";
	if (InputSettings->ImportMaterials) InputSettingsStr += "MATS ";
	if (InputSettings->ImportBlueprints) InputSettingsStr += "BPS ";
	if (InputSettings->ImportDecals) InputSettingsStr += "DECALS ";
	if (InputSettings->ImportLights) InputSettingsStr += "LIGHTS ";
	if (InputSettings->ImportMeshes) InputSettingsStr += "MESHES ";
	if (InputSettings->UseSubLevels) InputSettingsStr += "SUBLEVELS ";
	UE_LOG(LogTemp, Display, TEXT("Uiana: Called with %s"), *InputSettingsStr);
	AssetImporterComp = AssetImporter(Settings);
	MaterialImporterComp = MaterialImporter(Settings);
	BlueprintImporterComp = BlueprintImporter(Settings);
	// Open umaps JSON file and read the UMaps to store
	FString UMapJSON;
	Settings->UMapJsonPath.Path = FPaths::Combine(Settings->AssetsPath.Path, "umaps.json");
	FFileHelper::LoadFileToString(UMapJSON, *Settings->UMapJsonPath.Path);
	TSharedPtr<FJsonObject> JsonParsed = UianaHelpers::ParseJson(UMapJSON);
	if (!JsonParsed.IsValid() || !JsonParsed->TryGetStringArrayField(MapName, UMaps))
	{
		UE_LOG(LogTemp, Error, TEXT("UIANA: Failed to deserialize umaps for %s"), *MapName);
	}
}

void UUianaImporter::ImportMap()
{
	UBPFL::ChangeProjectSettings();
	UBPFL::ExecuteConsoleCommand("r.DefaultFeature.LightUnits 0");
	UBPFL::ExecuteConsoleCommand("r.DynamicGlobalIlluminationMethod 0");
	TArray<FString> umapPaths = AssetImporterComp.GetExtractedUmaps();
	UE_LOG(LogTemp, Display, TEXT("Found %d umaps"), umapPaths.Num());
	TArray<FString> levelPaths = {};
	if (!Settings->InputSettings->UseSubLevels)
	{
		levelPaths.Add(CreateNewLevel(Settings->Name));
	}
	// Clear level
	UEditorActorSubsystem* actorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	actorSubsystem->DestroyActors(actorSubsystem->GetAllLevelActors());
	if (Settings->InputSettings->ImportMaterials)
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Importing Textures!"));
		// Import textures first
		MaterialImporterComp.ImportMaterials();
	}
	if (Settings->InputSettings->ImportMeshes)
	{
		TSet<FString> meshes;
		TArray<FString> meshRawPaths;
		// TODO: Try using LoadFileToStringArrayWithPredicate() or WithLineVisitor to reduce code complexity with blacklist
		FFileHelper::LoadFileToStringArray(meshRawPaths, *FPaths::Combine(Settings->FolderPath.Path, "_assets_objects.txt"));
		for (FString meshRawPath : meshRawPaths)
		{
			if (IsBlacklisted(FPaths::GetCleanFilename(meshRawPath))) continue;
			FString temp, temp1, meshPath;
			meshRawPath.Replace(TEXT("\\"), TEXT("/")).Split(TEXT("/"), &temp, &temp1);
			if (temp.Equals("Engine")) continue;
			temp1.Split(TEXT("/"), &temp, &meshPath);
			meshPath = FPaths::Combine(Settings->ExportAssetsPath.Path, "Game", meshPath);
			meshes.Emplace(FPaths::SetExtension(meshPath, TEXT(".pskx")));
			// UE_LOG(LogTemp, Display, TEXT("Uiana: Found mesh with result path %s from raw path %s filtered to %s"), *FPaths::SetExtension(meshPath, TEXT(".pskx")), *meshRawPath, *meshPath);
		}
		UBPFL::ImportMeshes(meshes, Settings->ObjectsPath.Path);
	}
	if (Settings->InputSettings->ImportBlueprints)
	{
		TArray<FString> bpPaths;
		FFileManagerGeneric::Get().FindFiles(bpPaths, *Settings->ActorsPath.Path, TEXT(".json"));
		UE_LOG(LogTemp, Display, TEXT("Uiana: Found %d BPs to import!"), bpPaths.Num());
		UianaHelpers::AddPrefixPath(Settings->ActorsPath, bpPaths);
		BlueprintImporterComp.CreateBlueprints(Settings, bpPaths);
	}
	FScopedSlowTask UianaTask(umapPaths.Num(), LOCTEXT ("UianaTask", "Importing Map"));
	UianaTask.MakeDialog();
	// UWorld* world = GEditor->GetEditorWorldContext().World();
	UWorld* world = GEditor->PlayWorld;
	for (int i = umapPaths.Num() - 1; i >= 0; i--)
	{
		FString umapStr;
		FFileHelper::LoadFileToString(umapStr, *umapPaths[i]);
		TArray<TSharedPtr<FJsonValue>> umapData, umapFiltered;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(umapStr);
		if (!FJsonSerializer::Deserialize(JsonReader, umapData) || !umapData[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize umap %s"), *umapPaths[i]);
			continue;
		}
		FString umapName = FPaths::GetBaseFilename(umapPaths[i]);
		// TODO: Add level name to format text
		UianaTask.EnterProgressFrame(1, FText::Format(LOCTEXT("UianaTask", "Importing level {1}/{2}"), umapPaths.Num() - i, umapPaths.Num()));
		if (Settings->InputSettings->UseSubLevels)
		{
			levelPaths.Add(CreateNewLevel(umapName));
		}
		ImportUmap(umapData, umapName);
	}
	if (Settings->InputSettings->UseSubLevels)
	{
		UEditorLevelUtils::AddLevelsToWorld(world, levelPaths, ULevelStreamingAlwaysLoaded::StaticClass());
	}
	if (Settings->InputSettings->ImportMeshes)
	{
		TArray<FString> objPaths;
		FFileManagerGeneric::Get().FindFiles(objPaths, *Settings->ObjectsPath.Path, TEXT(".json"));
		UianaHelpers::AddPrefixPath(Settings->ObjectsPath, objPaths);
		for (const FString objPath : objPaths)
		{
			TArray<TSharedPtr<FJsonValue>> objData;
			FString objStr;
			FFileHelper::LoadFileToString(objStr, *objPath);
			const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(objStr);
			if (!FJsonSerializer::Deserialize(JsonReader, objData) || !objData[0].IsValid())
			{
				UE_LOG(LogScript, Warning, TEXT("Uiana: Failed to deserialize obj %s"), *objPath);
				continue;
			}
			for (TSharedPtr<FJsonValue> component : objData)
			{
				if (component->IsNull() || !component.IsValid()) continue;
				const TSharedPtr<FJsonObject> componentObj = component->AsObject();
				if (!componentObj->HasField("Properties")) continue;
				const TSharedPtr<FJsonObject> componentProps = componentObj->GetObjectField("Properties");
				if (componentObj->HasTypedField<EJson::String>("Type"))
				{
					UStaticMesh* mesh = Cast<UStaticMesh>(UEditorAssetLibrary::LoadAsset(FPaths::Combine("/Game/ValorantContent/Meshes/", componentObj->GetStringField("Name"))));
					if (mesh == nullptr)
					{
						UE_LOG(LogScript, Warning, TEXT("Uiana: Failed to import mesh to modify: %s"), *componentObj->GetStringField("Name"));
						continue;
					}
					if (componentObj->GetStringField("Type").Equals("StaticMesh"))
					{
						double lightmapRes = round(256 * Settings->InputSettings->LightmapResolutionMultiplier / 4) * 4;
						int lightmapCoord = 1;
						if (componentProps.IsValid() && componentProps->HasTypedField<EJson::Number>("LightMapCoordinateIndex"))
						{
							lightmapCoord = componentProps->GetIntegerField("LightMapCoordinateIndex");
						}
						if (componentProps.IsValid() && componentProps->HasTypedField<EJson::Number>("LightMapResolution"))
						{
							lightmapRes = round(componentProps->GetNumberField("LightMapResolution") * Settings->InputSettings->LightmapResolutionMultiplier / 4) * 4;
						}
						UE_LOG(LogTemp, Display, TEXT("Uiana: Mesh %s has lightmap resolution %d and lightmapCoord %d"), *componentObj->GetStringField("Name"), static_cast<int32>(lightmapRes), lightmapCoord);
						// mesh->SetLightMapResolution(lightmapRes);
						// mesh->SetLightMapCoordinateIndex(lightmapCoord);
						UianaHelpers::SetActorProperty(UStaticMesh::StaticClass(), mesh, "LightMapResolution", lightmapRes);
						UianaHelpers::SetActorProperty(UStaticMesh::StaticClass(), mesh, "LightMapCoordinateIndex", lightmapCoord);
					}
					if (componentObj->GetStringField("Type").Equals("BodySetup") && componentProps.IsValid() && componentProps->HasField("CollisionTraceFlag"))
					{
						// TODO: Verify this works vs setting the editor property!
						UBodySetup* bodySetup = mesh->GetBodySetup();
						bodySetup->CollisionTraceFlag = UianaHelpers::ParseCollisionTrace(componentProps->GetStringField("CollisionTraceFlag"));
						mesh->SetBodySetup(bodySetup);
					}
				}
			}
		}
	}
}



FString UUianaImporter::CreateNewLevel(const FString levelName)
{
	// Get initial name
	FString initialName, temp;
	levelName.Split(TEXT("_"), &initialName, &temp);
	if (initialName.Equals("")) initialName = levelName;
	TArray<FStringFormatArg> args = {initialName, levelName};
	const FString levelPath = FString::Format(TEXT("/Game/ValorantContent/Maps/{0}/{1}"), args);
	UE_LOG(LogTemp, Warning, TEXT("Uiana: Creating new level at path: %s"), *levelPath);
	UEditorAssetLibrary::LoadAsset(levelPath);
	ULevelEditorSubsystem* editorSubsystem = GEditor->GetEditorSubsystem<ULevelEditorSubsystem>();
	editorSubsystem->NewLevel(levelPath);
	return levelPath;
}

void UUianaImporter::ImportUmap(const TArray<TSharedPtr<FJsonValue>> umapData, const FString umapName)
{
	// Filter objects
	TArray<TSharedPtr<FJsonObject>> filteredObjs;
	TMap<FString, AActor*> bpMapping = {};
	int i = -1;
	for (const TSharedPtr<FJsonValue> objData : umapData)
	{
		i += 1;
		const TSharedPtr<FJsonObject> obj = objData.Get()->AsObject();
		FString objName;
		if (!obj.Get()->HasField("Properties")) objName = "None";
		else if (obj.Get()->GetObjectField("Properties")->HasTypedField<EJson::Object>("StaticMesh"))
		{
			objName = obj.Get()->GetObjectField("Properties")->GetObjectField("StaticMesh")->GetStringField("ObjectPath");
		}
		else if (obj.Get()->HasField("Outer"))
		{
			objName = obj.Get()->GetStringField("Outer");
		}
		else continue;
		objName = FPaths::GetCleanFilename(objName).ToLower();
		if (IsBlacklisted(objName)) continue;
		UianaHelpers::ObjectType objType = UianaHelpers::ParseObjectType(obj->GetStringField("Type"));
		if (Settings->InputSettings->ImportBlueprints && objType == UianaHelpers::ObjectType::Blueprint)
		{
			BlueprintImporterComp.ImportBlueprint(obj, bpMapping);
		}
		filteredObjs.Add(obj);
	}
	for (const TSharedPtr<FJsonObject> obj : filteredObjs)
	{
		UianaHelpers::ObjectType objType = UianaHelpers::ParseObjectType(obj->GetStringField("Type"));
		if (Settings->InputSettings->ImportMeshes && objType == UianaHelpers::ObjectType::Mesh)
		{
			ImportMesh(obj, umapName, bpMapping);
		}
		if (Settings->InputSettings->ImportDecals && objType == UianaHelpers::ObjectType::Decal)
		{
			ImportDecal(obj);
		}
		if (Settings->InputSettings->ImportLights && objType == UianaHelpers::ObjectType::Light)
		{
			ImportLight(obj);
		}
	}
}

void UUianaImporter::ImportMesh(const TSharedPtr<FJsonObject> obj, const FString umapName, const TMap<FString, AActor*> bpMapping)
{
	if (obj->HasField("Template"))
	{
		BlueprintImporter::FixActorBP(obj, bpMapping, Settings->InputSettings->ImportMaterials);
		return;
	}
	if (!obj->GetObjectField("Properties")->HasField("StaticMesh"))
	{
		return;
	}
	FTransform transform = UianaHelpers::GetTransformComponent(obj->GetObjectField("Properties"));
	UClass* meshActorClass = obj->HasField("PerInstanceSMData") && obj->GetStringField("Type").Contains("Instanced") ? AHismActorCPP::StaticClass() : AStaticMeshActor::StaticClass();
	AActor* meshActor = UEditorLevelLibrary::SpawnActorFromClass(meshActorClass, FVector::ZeroVector);
	meshActor->SetActorLabel(obj->GetStringField("Outer"));
	TArray<UObject*> meshActorObjects;
	meshActor->GetDefaultSubobjects(meshActorObjects);
	UHierarchicalInstancedStaticMeshComponent* meshInstancedObject = Cast<UHierarchicalInstancedStaticMeshComponent>(meshActorObjects.Last());
	UStaticMeshComponent* meshObject = Cast<UStaticMeshComponent>(meshActorObjects.Last());
	if (meshInstancedObject != nullptr) // Mesh is Instanced already
	{
		meshActor->SetFolderPath("Meshes/Static");
		const TArray<TSharedPtr<FJsonValue>> perInstanceData = obj->GetArrayField("PerInstanceSMData");
		for (const TSharedPtr<FJsonValue> instance : perInstanceData)
		{
			if (instance->AsObject()->HasField("Properties")) UE_LOG(LogTemp, Error, TEXT("Uiana: Need to use Properties field for PerInstanceSM Data!"));
			FTransform instanceTransform = UianaHelpers::GetTransformComponent(instance->AsObject());
			meshInstancedObject->AddInstance(instanceTransform);
		}
	}
	else
	{
		FString umapType = umapName.Contains("_VFX") ? "VFX" : "Static";
		meshActor->SetFolderPath(FName(*FPaths::Combine("Meshes", umapType)));
	}
	SetBPSettings(obj->GetObjectField("Properties"), meshObject); // TODO: If there are no bugs with this, rename to "SetActorSettings()"!
	meshObject->SetWorldTransform(transform);
	if (obj->HasField("LODData"))
	{
		const TArray<TSharedPtr<FJsonValue>> lodData = obj->GetArrayField("LODData");
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
		if (obj->GetObjectField("Properties")->HasField("StaticMesh"))
		{
			modelPath = FPaths::Combine(Settings->ExportAssetsPath.Path, FPaths::GetBaseFilename(obj->GetObjectField("Properties")->GetObjectField("StaticMesh")->GetStringField("ObjectPath"), false)) + ".pskx";
		}
		if (!vtxArray.IsEmpty())
		{
			UBPFL::PaintSMVertices(meshObject, vtxArray, modelPath);
		}
	}
	if (Settings->InputSettings->ImportMaterials && obj->GetObjectField("Properties")->HasField("OverrideMaterials"))
	{
		UianaHelpers::SetActorProperty(UStaticMeshComponent::StaticClass(), meshObject, "OverrideMaterials", MaterialImporterComp.CreateOverrideMaterials(obj));
	}
}

void UUianaImporter::ImportDecal(const TSharedPtr<FJsonObject> obj)
{
	if (obj->HasField("Template") || !UianaHelpers::HasTransformComponent(obj->GetObjectField("Properties"))) return;
	FTransform transform = UianaHelpers::GetTransformComponent(obj->GetObjectField("Properties"));
	ADecalActor* decalActor = Cast<ADecalActor>(UEditorLevelLibrary::SpawnActorFromClass(ADecalActor::StaticClass(), transform.GetTranslation(), transform.GetRotation().Rotator()));
	decalActor->SetFolderPath("Decals");
	decalActor->SetActorLabel(obj->GetStringField("Name"));
	decalActor->SetActorScale3D(transform.GetScale3D());
	// FString decalPath = FPaths::GetPath(obj->GetObjectField("DecalMaterial")->GetStringField("ObjectPath"));
	FString decalPath = obj->GetObjectField("Properties")->GetObjectField("DecalMaterial")->GetStringField("ObjectPath");
	UMaterialInstanceConstant* decalMat = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Materials/" + decalPath + "." + decalPath));
	decalActor->SetDecalMaterial(decalMat);
	SetBPSettings(obj->GetObjectField("Properties"), decalActor->GetDecal());
}

void UUianaImporter::ImportLight(const TSharedPtr<FJsonObject> obj)
{
	const FString lightType = obj->GetStringField("Type").Replace(TEXT("Component"), TEXT(""), ESearchCase::CaseSensitive);
	FTransform transform;
	if (UianaHelpers::HasTransformComponent(obj->GetObjectField("Properties")))
	{
		UE_LOG(LogTemp, Display, TEXT("Uiana: Light has TransformComponent, using it"));
		transform = UianaHelpers::GetTransformComponent(obj->GetObjectField("Properties"));
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Uiana: Light is using SceneTransform!"));
		transform = UianaHelpers::GetSceneTransformComponent(obj->GetObjectField("Properties"));
	}
	UClass* lightClass = FEditorClassUtils::GetClassFromString(lightType);
	UEditorActorSubsystem* EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	UE_LOG(LogTemp, Display, TEXT("Uiana: Creating light at position %d, %d, %d"), transform.GetTranslation().X, transform.GetTranslation().Y, transform.GetTranslation().Z);
	AActor* light = EditorActorSubsystem->SpawnActorFromClass(lightClass, transform.GetTranslation(), transform.GetRotation().Rotator());
	light->SetFolderPath(FName("Lights/" + lightType));
	light->SetActorLabel(obj->GetStringField("Name"));
	light->SetActorScale3D(transform.GetScale3D());
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
			// UianaHelpers::SetActorProperty(lightComponent->GetClass(), lightComponent, "Unbound", true);
			// UianaHelpers::SetActorProperty(lightComponent->GetClass(), lightComponent, "Priority", 1.0);
			SetBPSettings(obj->GetObjectField("Properties")->GetObjectField("Settings"), lightComponent);
		}
		SetBPSettings(obj->GetObjectField("Properties"), lightComponent);
		return;
	}
	const FProperty* settingsProp = PropertyAccessUtil::FindPropertyByName("Settings", lightComponent->GetClass());
	if (settingsProp != nullptr)
	{
		UianaHelpers::SetActorProperty(lightComponent->GetClass(), lightComponent, "Unbound", true);
		UianaHelpers::SetActorProperty(lightComponent->GetClass(), lightComponent, "Priority", 1.0);
		// UianaHelpers::SetActorProperty(lightComponent->GetClass(), lightComponent, "Unbound", true);
		// UianaHelpers::SetActorProperty(lightComponent->GetClass(), lightComponent, "Priority", 1.0);
		SetBPSettings(obj->GetObjectField("Properties")->GetObjectField("Settings"), lightComponent);
	}
	SetBPSettings(obj->GetObjectField("Properties"), lightComponent);
}

void UUianaImporter::SetBPSettings(const TSharedPtr<FJsonObject> bpProps, UActorComponent* bp)
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

bool UUianaImporter::IsBlacklisted(const FString itemName)
{
	for (const FString blacklistObj : Settings->BlacklistedObjs)
	{
		if (itemName.Contains(blacklistObj)) return true;
	}
	return false;
}

#undef LOCTEXT_NAMESPACE
