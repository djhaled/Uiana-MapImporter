#include "UianaImporter.h"

#include "BPFL.h"
#include "EditorAssetLibrary.h"
#include "EditorLevelLibrary.h"
#include "EditorLevelUtils.h"
#include "LevelEditorSubsystem.h"
#include "Engine/LevelStreamingAlwaysLoaded.h"
#include "Subsystems/EditorActorSubsystem.h"

#define LOCTEXT_NAMESPACE "UUianaImporter"

TArray<FString> UUianaImporter::UMaps = {};
UianaSettings UUianaImporter::Settings;
FAssetImporter UUianaImporter::AssetImporterComp;
MaterialImporter UUianaImporter::MaterialImporterComp;
MeshBlueprintImporter UUianaImporter::MeshBlueprintImporterComp;
FDecalLightImporter UUianaImporter::DecalLightImporterComp;

UUianaImporter::UUianaImporter()
{
	UE_LOG(LogTemp, Error, TEXT("Uiana: Must initialize importer with parameters!"));
}

void UUianaImporter::Initialize(FString MapName, UUianaCPPDataSettings* InputSettings)
{
	Settings = UianaSettings(MapName, InputSettings);
	AssetImporterComp = FAssetImporter(&Settings);
	MaterialImporterComp = MaterialImporter(&Settings);
	MeshBlueprintImporterComp = MeshBlueprintImporter(&Settings);
	DecalLightImporterComp = FDecalLightImporter(&Settings);
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
		}
		UBPFL::ImportMeshes(meshes, Settings.ObjectsPath.Path);
	}
	if (Settings.ImportBlueprints)
	{
		TArray<FString> bpPaths;
		FFileManagerGeneric::Get().FindFiles(bpPaths, *Settings.ActorsPath.Path, TEXT(".json"));
		UE_LOG(LogTemp, Display, TEXT("Uiana: Found %d BPs to import!"), bpPaths.Num());
		UianaHelpers::AddPrefixPath(Settings.ActorsPath, bpPaths);
		MeshBlueprintImporterComp.CreateBlueprints(bpPaths);
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

FString UUianaImporter::CreateNewLevel(const FString LevelName)
{
	// Get initial name
	FString initialName, temp;
	LevelName.Split(TEXT("_"), &initialName, &temp);
	if (initialName.Equals("")) initialName = LevelName;
	TArray<FStringFormatArg> args = {initialName, LevelName};
	const FString levelPath = FString::Format(TEXT("/Game/ValorantContent/Maps/{0}/{1}"), args);
	UE_LOG(LogTemp, Warning, TEXT("Uiana: Creating new level at path: %s"), *levelPath);
	UEditorAssetLibrary::LoadAsset(levelPath);
	ULevelEditorSubsystem* editorSubsystem = GEditor->GetEditorSubsystem<ULevelEditorSubsystem>();
	editorSubsystem->NewLevel(levelPath);
	return levelPath;
}

void UUianaImporter::ImportUmap(const TArray<TSharedPtr<FJsonValue>> UmapData, const FString UmapName)
{
	// Filter objects
	TArray<TSharedPtr<FJsonObject>> filteredObjs;
	TMap<FString, AActor*> bpMapping = {};
	int i = -1;
	for (const TSharedPtr<FJsonValue> objData : UmapData)
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
		UianaHelpers::EObjectType objType = UianaHelpers::ParseObjectType(obj->GetStringField("Type"));
		if (Settings.ImportBlueprints && objType == UianaHelpers::EObjectType::Blueprint)
		{
			MeshBlueprintImporterComp.ImportBlueprint(obj, bpMapping);
		}
		filteredObjs.Add(obj);
	}
	for (const TSharedPtr<FJsonObject> obj : filteredObjs)
	{
		UianaHelpers::EObjectType objType = UianaHelpers::ParseObjectType(obj->GetStringField("Type"));
		if (Settings.ImportMeshes && objType == UianaHelpers::EObjectType::Mesh)
		{
			MeshBlueprintImporterComp.ImportMesh(obj, UmapName, bpMapping);
		}
		if (Settings.ImportDecals && objType == UianaHelpers::EObjectType::Decal)
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: Importing decal!"));
			DecalLightImporterComp.ImportDecal(obj);
		}
		if (Settings.ImportLights && objType == UianaHelpers::EObjectType::Light)
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: Importing light %s!"), *obj.Get()->GetStringField("Name"));
			DecalLightImporterComp.ImportLight(obj);
		}
	}
}

bool UUianaImporter::IsBlacklisted(const FString ItemName)
{
	for (const FString blacklistObj : Settings.BlacklistedObjs)
	{
		if (ItemName.Contains(blacklistObj)) return true;
	}
	return false;
}

#undef LOCTEXT_NAMESPACE

