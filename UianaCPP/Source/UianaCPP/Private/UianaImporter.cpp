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
UianaSettings UUianaImporter::Settings;
AssetImporter UUianaImporter::AssetImporterComp;
MaterialImporter UUianaImporter::MaterialImporterComp;
BlueprintImporter UUianaImporter::BlueprintImporterComp;

UUianaImporter::UUianaImporter()
{
	UE_LOG(LogTemp, Error, TEXT("Uiana: Must initialize importer with parameters!"));
}

void UUianaImporter::Initialize(FString MapName, UUianaCPPDataSettings* InputSettings)
{
	Settings = UianaSettings(MapName, InputSettings);
	AssetImporterComp = AssetImporter(&Settings);
	MaterialImporterComp = MaterialImporter(&Settings);
	BlueprintImporterComp = BlueprintImporter(&Settings);
	// Open umaps JSON file and read the UMaps to store
	FString UMapJSON;
	Settings.UMapJsonPath.Path = FPaths::Combine(Settings.AssetsPath.Path, "umaps.json");
	FFileHelper::LoadFileToString(UMapJSON, *Settings.UMapJsonPath.Path);
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
	if (!Settings.UseSubLevels)
	{
		levelPaths.Add(CreateNewLevel(Settings.Name));
	}
	// Clear level
	UEditorActorSubsystem* actorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	actorSubsystem->DestroyActors(actorSubsystem->GetAllLevelActors());
	if (Settings.ImportMaterials)
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Importing Textures!"));
		// Import textures first
		MaterialImporterComp.ImportMaterials();
	}
	if (Settings.ImportMeshes)
	{
		TSet<FString> meshes;
		TArray<FString> meshRawPaths;
		// TODO: Try using LoadFileToStringArrayWithPredicate() or WithLineVisitor to reduce code complexity with blacklist
		FFileHelper::LoadFileToStringArray(meshRawPaths, *FPaths::Combine(Settings.FolderPath.Path, "_assets_objects.txt"));
		for (FString meshRawPath : meshRawPaths)
		{
			if (IsBlacklisted(FPaths::GetCleanFilename(meshRawPath))) continue;
			FString temp, temp1, meshPath;
			meshRawPath.Replace(TEXT("\\"), TEXT("/")).Split(TEXT("/"), &temp, &temp1);
			if (temp.Equals("Engine")) continue;
			temp1.Split(TEXT("/"), &temp, &meshPath);
			meshPath = FPaths::Combine(Settings.ExportAssetsPath.Path, "Game", meshPath);
			meshes.Emplace(FPaths::SetExtension(meshPath, TEXT(".pskx")));
			// UE_LOG(LogTemp, Display, TEXT("Uiana: Found mesh with result path %s from raw path %s filtered to %s"), *FPaths::SetExtension(meshPath, TEXT(".pskx")), *meshRawPath, *meshPath);
		}
		UBPFL::ImportMeshes(meshes, Settings.ObjectsPath.Path);
	}
	if (Settings.ImportBlueprints)
	{
		TArray<FString> bpPaths;
		FFileManagerGeneric::Get().FindFiles(bpPaths, *Settings.ActorsPath.Path, TEXT(".json"));
		UE_LOG(LogTemp, Display, TEXT("Uiana: Found %d BPs to import!"), bpPaths.Num());
		UianaHelpers::AddPrefixPath(Settings.ActorsPath, bpPaths);
		BlueprintImporterComp.CreateBlueprints(bpPaths);
	}
	FScopedSlowTask UianaTask(umapPaths.Num(), LOCTEXT ("UianaTask", "Importing Map"));
	UianaTask.MakeDialog();
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
		if (Settings.UseSubLevels)
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: Attempting to create level for umap %s"), *umapName);
			levelPaths.Add(CreateNewLevel(umapName));
			UE_LOG(LogTemp, Display, TEXT("Uiana: Exiting creating level for umap %s"), *umapName);
		}
		UE_LOG(LogTemp, Display, TEXT("Uiana: Attempting to import umap %s"), *umapName);
		ImportUmap(umapData, umapName);
		UE_LOG(LogTemp, Display, TEXT("Uiana: Finished importing assets from umap %s"), *umapName);
		if (Settings.UseSubLevels)
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: Saving level for umap %s"), *umapName);
			if (!UEditorLevelLibrary::SaveCurrentLevel()) UE_LOG(LogTemp, Error, TEXT("Uiana: Failed to save level for umap %s!"), *umapName);
		}
		UE_LOG(LogTemp, Display, TEXT("Uiana: Finished level for umap %s"), *umapName);
	}
	if (Settings.UseSubLevels)
	{
		UE_LOG(LogTemp, Display, TEXT("Uiana: Adding all levels to world!"));
		UWorld* world = UEditorLevelLibrary::GetEditorWorld();
		if (world == nullptr)
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: GetEditorWorld is not valid world, trying GetEditorWorldContext()!"));
			world = GEditor->GetEditorWorldContext().World();
		}
		if (world == nullptr)
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: GetEditorWorldContext is not valid world, trying GetEditorSubsystem()!"));
			world = GEditor->GetEditorSubsystem<ULevelEditorSubsystem>()->GetWorld();
		}
		if (world == nullptr)
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: GetEditorSubsystem is not valid world, trying all GetEditorWorldContext worlds()!"));
			UWorld* PIE = nullptr;
			UWorld* GamePreview = nullptr;
			for (FWorldContext const& Context : GEngine->GetWorldContexts())
			{
				switch (Context.WorldType)
				{
				case EWorldType::PIE:
					PIE = Context.World();
					break;
				case EWorldType::GamePreview:
					GamePreview = Context.World();
					break;
				}
			}
			if (PIE)
			{
				world = PIE;
			}
			else if (GamePreview)
			{
				world = GamePreview;
			}
		}
		
		if (world == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Uiana: World pointer is null!"));
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: World pointer not null, saving all levels!"));
		}
		// UEditorLevelUtils::AddLevelsToWorld(world, levelPaths, ULevelStreamingAlwaysLoaded::StaticClass());
		for (const FString levelPath : levelPaths)
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: Adding level %s to world!"), *levelPath);
			UEditorLevelUtils::AddLevelToWorld(world, *levelPath, ULevelStreamingAlwaysLoaded::StaticClass());
		}
	}
	if (Settings.ImportMeshes)
	{
		TArray<FString> objPaths;
		FFileManagerGeneric::Get().FindFiles(objPaths, *Settings.ObjectsPath.Path, TEXT(".json"));
		UianaHelpers::AddPrefixPath(Settings.ObjectsPath, objPaths);
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
				// if (!componentObj->HasField("Properties")) continue; // Cannot just assume every static mesh has Properties!
				const TSharedPtr<FJsonObject> componentProps = componentObj->GetObjectField("Properties");
				if (componentObj->HasTypedField<EJson::String>("Type"))
				{
					UStaticMesh* mesh = Cast<UStaticMesh>(UEditorAssetLibrary::LoadAsset(FPaths::Combine("/Game/ValorantContent/Meshes/", componentObj->GetStringField("Name"))));
					if (mesh == nullptr)
					{
						if (!componentObj->GetStringField("Name").Contains("BodySetup")) UE_LOG(LogScript, Warning, TEXT("Uiana: Failed to import mesh to modify: %s"), *componentObj->GetStringField("Name"));
						continue;
					}
					if (componentObj->GetStringField("Type").Equals("StaticMesh"))
					{
						double lightmapRes = round(256 * Settings.LightmapResolutionMultiplier / 4) * 4;
						int lightmapCoord = 1;
						if (componentProps.IsValid() && componentProps->HasTypedField<EJson::Number>("LightMapCoordinateIndex"))
						{
							lightmapCoord = componentProps->GetIntegerField("LightMapCoordinateIndex");
						}
						if (componentProps.IsValid() && componentProps->HasTypedField<EJson::Number>("LightMapResolution"))
						{
							lightmapRes = round(componentProps->GetNumberField("LightMapResolution") * Settings.LightmapResolutionMultiplier / 4) * 4;
						}
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
		if (!obj.IsValid()) UE_LOG(LogTemp, Error, TEXT("Uiana: Umap obj is not object!"));
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
		if (Settings.ImportBlueprints && objType == UianaHelpers::ObjectType::Blueprint)
		{
			BlueprintImporterComp.ImportBlueprint(obj, bpMapping);
		}
		filteredObjs.Add(obj);
	}
	for (const TSharedPtr<FJsonObject> obj : filteredObjs)
	{
		UianaHelpers::ObjectType objType = UianaHelpers::ParseObjectType(obj->GetStringField("Type"));
		if (Settings.ImportMeshes && objType == UianaHelpers::ObjectType::Mesh)
		{
			ImportMesh(obj, umapName, bpMapping);
		}
		if (Settings.ImportDecals && objType == UianaHelpers::ObjectType::Decal)
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: Importing decal!"));
			ImportDecal(obj);
		}
		if (Settings.ImportLights && objType == UianaHelpers::ObjectType::Light)
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: Importing light %s!"), *obj.Get()->GetStringField("Name"));
			ImportLight(obj);
		}
	}
}

void UUianaImporter::ImportMesh(const TSharedPtr<FJsonObject> obj, const FString umapName, const TMap<FString, AActor*> bpMapping)
{
	if (obj->HasField("Template"))
	{
		BlueprintImporter::FixActorBP(obj, bpMapping, Settings.ImportMaterials);
		return;
	}
	if (!obj->GetObjectField("Properties")->HasField("StaticMesh") || !UianaHelpers::HasTransformComponent(obj->GetObjectField("Properties")))
	{
		return;
	}
	const bool isInstanced = obj->HasField("PerInstanceSMData") && obj->GetStringField("Type").Contains("Instanced");
	FTransform transform = UianaHelpers::GetTransformComponent(obj->GetObjectField("Properties"));
	UClass* meshActorClass = isInstanced ? AHismActorCPP::StaticClass() : AStaticMeshActor::StaticClass();
	AActor* meshActor = UEditorLevelLibrary::SpawnActorFromClass(meshActorClass, FVector::ZeroVector);
	meshActor->SetActorLabel(obj->GetStringField("Outer"));
	TArray<UObject*> meshActorObjects;
	meshActor->GetDefaultSubobjects(meshActorObjects);
	UHierarchicalInstancedStaticMeshComponent* meshInstancedObject = Cast<UHierarchicalInstancedStaticMeshComponent>(meshActorObjects.Last());
	UStaticMeshComponent* meshObject;
	if (isInstanced)
	{
		meshActor->SetFolderPath("Meshes/Instanced");
		const TArray<TSharedPtr<FJsonValue>> perInstanceData = obj->GetArrayField("PerInstanceSMData");
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
		FString umapType = umapName.Contains("_VFX") ? "VFX" : "Static";
		meshActor->SetFolderPath(FName(*FPaths::Combine("Meshes", umapType)));
		meshObject = Cast<UStaticMeshComponent>(meshActorObjects.Last());
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
			modelPath = FPaths::Combine(
					Settings.ExportAssetsPath.Path,
					FPaths::GetBaseFilename(
						obj->GetObjectField("Properties")->GetObjectField("StaticMesh")->GetStringField("ObjectPath"),
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
	if (Settings.ImportMaterials && obj->GetObjectField("Properties")->HasField("OverrideMaterials"))
	{
		TArray<UMaterialInterface*> OverrideMats = MaterialImporterComp.CreateOverrideMaterials(obj);
		if (!OverrideMats.IsEmpty())
		{
			UianaHelpers::SetActorProperty(UStaticMeshComponent::StaticClass(), meshObject, "OverrideMaterials", OverrideMats);	
		}
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
	FString decalName = FPaths::GetBaseFilename(obj->GetObjectField("Properties")->GetObjectField("DecalMaterial")->GetStringField("ObjectPath"));
	UMaterialInstanceConstant* decalMat = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Materials/" + decalName + "." + decalName));
	decalActor->SetDecalMaterial(decalMat);
	SetBPSettings(obj->GetObjectField("Properties"), decalActor->GetDecal());
}

void UUianaImporter::ImportLight(const TSharedPtr<FJsonObject> obj)
{
	const FString lightType = obj->GetStringField("Type").Replace(TEXT("Component"), TEXT(""), ESearchCase::CaseSensitive);
	FTransform transform;
	if (UianaHelpers::HasTransformComponent(obj->GetObjectField("Properties")))
	{
		transform = UianaHelpers::GetTransformComponent(obj->GetObjectField("Properties"));
	}
	else
	{
		transform = UianaHelpers::GetSceneTransformComponent(obj->GetObjectField("Properties"));
	}
	UClass* lightClass = FEditorClassUtils::GetClassFromString(lightType);
	UEditorActorSubsystem* EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
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
		FProperty* objectProp = PropertyAccessUtil::FindPropertyByName(propName, bp->GetClass());
		if (objectProp == nullptr) continue;
		const EJson propType = propValue.Get()->Type;
		if (propType == EJson::Number)
		{
			if (const FFloatProperty* floatProp = CastField<FFloatProperty>(objectProp))
			{
				if (prop.Key.Equals("InfluenceRadius") && propValue.Get()->AsNumber() == 0)
				{
					floatProp->SetPropertyValue_InContainer(bp, 14680.0);
				}
				else
				{
					floatProp->SetPropertyValue_InContainer(bp, prop.Value.Get()->AsNumber());
				}
			}
			else if (const FIntProperty* intProp = CastField<FIntProperty>(objectProp))
			{
				intProp->SetPropertyValue_InContainer(bp, prop.Value.Get()->AsNumber());
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
				boolProp->SetPropertyValue_InContainer(bp, prop.Value.Get()->AsBool());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to cast %s into bool prop!"), *prop.Key);
			}
		}
		else if (propType == EJson::Object)
		{
			if (prop.Key.Equals("IESTexture"))
			{
				FString temp, newTextureName;
				propValue.Get()->AsObject()->GetStringField("ObjectName").Split("_", &temp, &newTextureName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
				FString assetPath = "/UianaCPP/IESProfiles/" + newTextureName + "." + newTextureName;
				UTexture* newTexture = Cast<UTexture>(UEditorAssetLibrary::LoadAsset(assetPath));
				UianaHelpers::SetActorProperty<UTexture*>(bp->GetClass(), bp, prop.Key, newTexture);
			}
			else if (prop.Key.Equals("Cubemap"))
			{
				FString newCubemapName = propValue.Get()->AsObject()->GetStringField("ObjectName").Replace(TEXT("TextureCube "), TEXT(""));
				FString assetPath = "/UianaCPP/CubeMaps/" + newCubemapName + "." + newCubemapName;
				// TODO: Convert all static_cast with UObjects to Cast<>()
				UTextureCube* newCube = Cast<UTextureCube, UObject>(UEditorAssetLibrary::LoadAsset(assetPath));
				UianaHelpers::SetActorProperty<UTextureCube*>(bp->GetClass(), bp, prop.Key, newCube);
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
				}
			}
			else if (prop.Key.Equals("BoxExtent"))
			{
				if (objectProp->GetClass()->GetName().Equals("StructProperty")) UE_LOG(LogTemp, Error, TEXT("Uiana: Mesh Setting BoxExtent already handled by Struct logic!"));
				FVector vec;
				const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
				vec.X = obj->GetNumberField("X");
				vec.Y = obj->GetNumberField("Y");
				vec.Z = obj->GetNumberField("Z");
				UianaHelpers::SetActorProperty<FVector>(bp->GetClass(), bp, "BoxExtent", vec);
			}
			else if (objectProp->GetClass()->GetName().Equals("StructProperty"))
			{
				const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
				if (const FStructProperty* colorValues = CastField<FStructProperty>(objectProp))
				{
					FString OutputString;
					TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
					FJsonSerializer::Serialize(propValue.Get()->AsObject().ToSharedRef(), Writer);
					UScriptStruct* Class = nullptr;
					FString ClassName = objectProp->GetCPPType().TrimChar('F');
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
					void* structSettingsAddr = objectProp->ContainerPtrToValuePtr<void>(bp);
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
								if (FBoolProperty* overrideProp = Cast<FBoolProperty>(overrideFlagProp))
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
			UianaHelpers::SetActorProperty<TArray<UMaterialInstanceConstant*>>(bp->GetClass(), bp, prop.Key, overrideMats);
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
		}
	}
}

bool UUianaImporter::IsBlacklisted(const FString itemName)
{
	for (const FString blacklistObj : Settings.BlacklistedObjs)
	{
		if (itemName.Contains(blacklistObj)) return true;
	}
	return false;
}

#undef LOCTEXT_NAMESPACE

