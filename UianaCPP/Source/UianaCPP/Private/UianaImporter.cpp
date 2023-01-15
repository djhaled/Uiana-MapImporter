#include "UianaImporter.h"

#include "BPFL.h"
#include "EditorAssetLibrary.h"
#include "EditorLevelLibrary.h"
#include "EditorLevelUtils.h"
#include "Engine/LevelStreamingAlwaysLoaded.h"
#if ENGINE_MAJOR_VERSION == 5
#include "Subsystems/EditorActorSubsystem.h"
#include "LevelEditorSubsystem.h"
#endif

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

void UUianaImporter::Initialize(const FString MapName, UUianaCPPDataSettings* InputSettings)
{
	Settings = UianaSettings(MapName, InputSettings);
	AssetImporterComp = FAssetImporter(&Settings);
	MaterialImporterComp = MaterialImporter(&Settings);
	MeshBlueprintImporterComp = MeshBlueprintImporter(&Settings);
	DecalLightImporterComp = FDecalLightImporter(&Settings);
	// Open umaps JSON file and read the UMaps to store
	FString UMapJSON;
	Settings.UMapJsonPath.Path = FPaths::Combine(Settings.AssetsPath.Path, TEXT("umaps.json"));
	FFileHelper::LoadFileToString(UMapJSON, *Settings.UMapJsonPath.Path);
	const TSharedPtr<FJsonObject> JsonParsed = UianaHelpers::ParseJson(UMapJSON);
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
	TArray<FString> UmapPaths = AssetImporterComp.GetExtractedUmaps();
	UE_LOG(LogTemp, Display, TEXT("Found %d umaps"), UmapPaths.Num());
	TArray<FString> LevelPaths = {};
	if (!Settings.UseSubLevels)
	{
		LevelPaths.Add(CreateNewLevel(Settings.Name));
	}
	// Clear level
#if ENGINE_MAJOR_VERSION == 5
	UEditorActorSubsystem* ActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	ActorSubsystem->DestroyActors(ActorSubsystem->GetAllLevelActors());
#else
	TArray<AActor*> CurrentActors = UEditorLevelLibrary::GetAllLevelActors();
	for (AActor* CurrentActor : CurrentActors) UEditorLevelLibrary::DestroyActor(CurrentActor);
#endif
	if (Settings.ImportMaterials)
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Importing Textures!"));
		// Import textures first
		MaterialImporterComp.ImportMaterials();
	}
	if (Settings.ImportMeshes)
	{
		TSet<FString> Meshes;
		TArray<FString> MeshRawPaths;
		// TODO: Try using LoadFileToStringArrayWithPredicate() or WithLineVisitor to reduce code complexity with blacklist
		FFileHelper::LoadFileToStringArray(MeshRawPaths, *FPaths::Combine(Settings.FolderPath.Path, TEXT("_assets_objects.txt")));
		for (FString MeshRawPath : MeshRawPaths)
		{
			if (IsBlacklisted(FPaths::GetCleanFilename(MeshRawPath))) continue;
			FString Temp, Temp1, MeshPath;
			MeshRawPath.Replace(TEXT("\\"), TEXT("/")).Split(TEXT("/"), &Temp, &Temp1);
			if (Temp.Equals("Engine")) continue;
			Temp1.Split(TEXT("/"), &Temp, &MeshPath);
			MeshPath = FPaths::Combine(Settings.ExportAssetsPath.Path, TEXT("Game"), MeshPath);
			Meshes.Emplace(FPaths::SetExtension(MeshPath, TEXT(".pskx")));
		}
		UBPFL::ImportMeshes(Meshes, Settings.ObjectsPath.Path);
	}
	if (Settings.ImportBlueprints)
	{
		TArray<FString> BPPaths;
		FFileManagerGeneric::Get().FindFiles(BPPaths, *Settings.ActorsPath.Path, TEXT(".json"));
		UE_LOG(LogTemp, Display, TEXT("Uiana: Found %d BPs to import!"), BPPaths.Num());
		UianaHelpers::AddPrefixPath(Settings.ActorsPath, BPPaths);
		MeshBlueprintImporterComp.CreateBlueprints(BPPaths);
	}
	FScopedSlowTask UianaTask(UmapPaths.Num(), LOCTEXT ("UianaTask", "Importing Map"));
	UianaTask.MakeDialog();
	for (int i = UmapPaths.Num() - 1; i >= 0; i--)
	{
		FString UmapStr;
		FFileHelper::LoadFileToString(UmapStr, *UmapPaths[i]);
		TArray<TSharedPtr<FJsonValue>> UmapData, UmapFiltered;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(UmapStr);
		if (!FJsonSerializer::Deserialize(JsonReader, UmapData) || !UmapData[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize umap %s"), *UmapPaths[i]);
			continue;
		}
		FString UmapName = FPaths::GetBaseFilename(UmapPaths[i]);
		// TODO: Add level name to format text
		UianaTask.EnterProgressFrame(1, FText::Format(LOCTEXT("UianaTask", "Importing level {1}/{2}"), UmapPaths.Num() - i, UmapPaths.Num()));
		if (Settings.UseSubLevels)
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: Attempting to create level for umap %s"), *UmapName);
			LevelPaths.Add(CreateNewLevel(UmapName));
			UE_LOG(LogTemp, Display, TEXT("Uiana: Exiting creating level for umap %s"), *UmapName);
		}
		UE_LOG(LogTemp, Display, TEXT("Uiana: Attempting to import umap %s"), *UmapName);
		ImportUmap(UmapData, UmapName);
		UE_LOG(LogTemp, Display, TEXT("Uiana: Finished importing assets from umap %s"), *UmapName);
		if (Settings.UseSubLevels)
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: Saving level for umap %s"), *UmapName);
			if (!UEditorLevelLibrary::SaveCurrentLevel()) UE_LOG(LogTemp, Error, TEXT("Uiana: Failed to save level for umap %s!"), *UmapName);
		}
		UE_LOG(LogTemp, Display, TEXT("Uiana: Finished level for umap %s"), *UmapName);
	}
	if (Settings.UseSubLevels)
	{
		UE_LOG(LogTemp, Display, TEXT("Uiana: Adding all levels to world!"));
		UWorld* World = UEditorLevelLibrary::GetEditorWorld();
		if (World == nullptr)
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: GetEditorWorld is not valid world, trying GetEditorWorldContext()!"));
			World = GEditor->GetEditorWorldContext().World();
		}
		// if (World == nullptr)
		// {
		// 	UE_LOG(LogTemp, Display, TEXT("Uiana: GetEditorWorldContext is not valid world, trying GetEditorSubsystem()!"));
		// 	World = GEditor->GetEditorSubsystem<ULevelEditorSubsystem>()->GetWorld();
		// }
		if (World == nullptr)
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
				World = PIE;
			}
			else if (GamePreview)
			{
				World = GamePreview;
			}
		}
		
		if (World == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Uiana: World pointer is null!"));
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: World pointer not null, saving all levels!"));
		}
		for (const FString LevelPath : LevelPaths)
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: Adding level %s to world!"), *LevelPath);
			UEditorLevelUtils::AddLevelToWorld(World, *LevelPath, ULevelStreamingAlwaysLoaded::StaticClass());
		}
	}
	if (Settings.ImportMeshes)
	{
		TArray<FString> ObjPaths;
		FFileManagerGeneric::Get().FindFiles(ObjPaths, *Settings.ObjectsPath.Path, TEXT(".json"));
		UianaHelpers::AddPrefixPath(Settings.ObjectsPath, ObjPaths);
		for (const FString ObjPath : ObjPaths)
		{
			TArray<TSharedPtr<FJsonValue>> ObjData;
			FString ObjStr;
			FFileHelper::LoadFileToString(ObjStr, *ObjPath);
			const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(ObjStr);
			if (!FJsonSerializer::Deserialize(JsonReader, ObjData) || !ObjData[0].IsValid())
			{
				UE_LOG(LogScript, Warning, TEXT("Uiana: Failed to deserialize obj %s"), *ObjPath);
				continue;
			}
			for (TSharedPtr<FJsonValue> Component : ObjData)
			{
				if (Component->IsNull() || !Component.IsValid()) continue;
				const TSharedPtr<FJsonObject> ComponentObj = Component->AsObject();
				const TSharedPtr<FJsonObject> ComponentProps = ComponentObj->GetObjectField("Properties");
				if (ComponentObj->HasTypedField<EJson::String>("Type"))
				{
					UStaticMesh* Mesh = Cast<UStaticMesh>(UEditorAssetLibrary::LoadAsset(FPaths::Combine(TEXT("/Game/ValorantContent/Meshes/"), ComponentObj->GetStringField("Name"))));
					if (Mesh == nullptr)
					{
						if (!ComponentObj->GetStringField("Name").Contains("BodySetup")) UE_LOG(LogScript, Warning, TEXT("Uiana: Failed to import mesh to modify: %s"), *ComponentObj->GetStringField("Name"));
						continue;
					}
					if (ComponentObj->GetStringField("Type").Equals("StaticMesh"))
					{
						double LightmapRes = round(256 * Settings.LightmapResolutionMultiplier / 4) * 4;
						int LightmapCoord = 1;
						if (ComponentProps.IsValid() && ComponentProps->HasTypedField<EJson::Number>("LightMapCoordinateIndex"))
						{
							LightmapCoord = ComponentProps->GetIntegerField("LightMapCoordinateIndex");
						}
						if (ComponentProps.IsValid() && ComponentProps->HasTypedField<EJson::Number>("LightMapResolution"))
						{
							LightmapRes = round(ComponentProps->GetNumberField("LightMapResolution") * Settings.LightmapResolutionMultiplier / 4) * 4;
						}
						UianaHelpers::SetActorProperty(UStaticMesh::StaticClass(), Mesh, "LightMapResolution", LightmapRes);
						UianaHelpers::SetActorProperty(UStaticMesh::StaticClass(), Mesh, "LightMapCoordinateIndex", LightmapCoord);
					}
					if (ComponentObj->GetStringField("Type").Equals("BodySetup") && ComponentProps.IsValid() && ComponentProps->HasField("CollisionTraceFlag"))
					{
						// TODO: Verify this works vs setting the editor property!
						UBodySetup* BodySetup = Mesh->GetBodySetup();
						BodySetup->CollisionTraceFlag = UianaHelpers::ParseCollisionTrace(ComponentProps->GetStringField("CollisionTraceFlag"));
						Mesh->SetBodySetup(BodySetup);
					}
				}
			}
		}
	}
}

FString UUianaImporter::CreateNewLevel(const FString LevelName)
{
	// Get initial name
	FString InitialName, Temp;
	LevelName.Split(TEXT("_"), &InitialName, &Temp);
	if (InitialName.Equals("")) InitialName = LevelName;
	TArray<FStringFormatArg> Args = {InitialName, LevelName};
	const FString LevelPath = FString::Format(TEXT("/Game/ValorantContent/Maps/{0}/{1}"), Args);
	UE_LOG(LogTemp, Warning, TEXT("Uiana: Creating new level at path: %s"), *LevelPath);
	UEditorAssetLibrary::LoadAsset(LevelPath);
#if ENGINE_MAJOR_VERSION == 5
	ULevelEditorSubsystem* EditorSubsystem = GEditor->GetEditorSubsystem<ULevelEditorSubsystem>();
	EditorSubsystem->NewLevel(LevelPath);
#else
	UEditorLevelLibrary::NewLevel(LevelPath);
#endif
	return LevelPath;
}

void UUianaImporter::ImportUmap(const TArray<TSharedPtr<FJsonValue>> UmapData, const FString UmapName)
{
	// Filter objects
	TArray<TSharedPtr<FJsonObject>> FilteredObjs;
	TMap<FString, AActor*> BPMapping = {};
	int i = -1;
	for (const TSharedPtr<FJsonValue> objData : UmapData)
	{
		i += 1;
		const TSharedPtr<FJsonObject> Obj = objData.Get()->AsObject();
		if (!Obj.IsValid()) UE_LOG(LogTemp, Error, TEXT("Uiana: Umap obj is not object!"));
		FString ObjName;
		if (!Obj.Get()->HasField("Properties")) ObjName = TEXT("None");
		else if (Obj.Get()->GetObjectField("Properties")->HasTypedField<EJson::Object>("StaticMesh"))
		{
			ObjName = Obj.Get()->GetObjectField("Properties")->GetObjectField("StaticMesh")->GetStringField("ObjectPath");
		}
		else if (Obj.Get()->HasField("Outer"))
		{
			ObjName = Obj.Get()->GetStringField("Outer");
		}
		else continue;
		ObjName = FPaths::GetCleanFilename(ObjName).ToLower();
		if (IsBlacklisted(ObjName)) continue;
		UianaHelpers::EObjectType ObjType = UianaHelpers::ParseObjectType(Obj->GetStringField("Type"));
		if (Settings.ImportBlueprints && ObjType == UianaHelpers::EObjectType::Blueprint)
		{
			MeshBlueprintImporterComp.ImportBlueprint(Obj, BPMapping);
		}
		FilteredObjs.Add(Obj);
	}
	for (const TSharedPtr<FJsonObject> Obj : FilteredObjs)
	{
		UianaHelpers::EObjectType ObjType = UianaHelpers::ParseObjectType(Obj->GetStringField("Type"));
		if (Settings.ImportMeshes && ObjType == UianaHelpers::EObjectType::Mesh)
		{
			MeshBlueprintImporterComp.ImportMesh(Obj, UmapName, BPMapping);
		}
		if (Settings.ImportDecals && ObjType == UianaHelpers::EObjectType::Decal)
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: Importing decal!"));
			DecalLightImporterComp.ImportDecal(Obj);
		}
		if (Settings.ImportLights && ObjType == UianaHelpers::EObjectType::Light)
		{
			UE_LOG(LogTemp, Display, TEXT("Uiana: Importing light %s!"), *Obj.Get()->GetStringField("Name"));
			DecalLightImporterComp.ImportLight(Obj);
		}
	}
}

bool UUianaImporter::IsBlacklisted(const FString ItemName)
{
	for (const FString BlacklistedObj : Settings.BlacklistedObjs)
	{
		if (ItemName.Contains(BlacklistedObj)) return true;
	}
	return false;
}

#undef LOCTEXT_NAMESPACE

