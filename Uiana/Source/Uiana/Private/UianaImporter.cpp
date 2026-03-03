#include "UianaImporter.h"

#include "BPFL.h"
#include "HismActor.h"
#include "UianaDataSettings.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Components/DecalComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DecalActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/RectLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SpotLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Blueprint.h"
#include "Editor.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialEditingLibrary.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/PackageName.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/SavePackage.h"

namespace
{
	static const FString GDefaultAES = TEXT("0x4BE71AF2459CF83899EC9DC2CB60E22AC4B3047E0211034BBABE9D174C069DD6");

	struct FMapPaths
	{
		FString Root;
		FString Actors;
		FString Materials;
		FString MaterialsOverride;
		FString Objects;
		FString UMaps;
	};

	FString NormalizeMapName(const FString& InMapName)
	{
		FString Name = InMapName.ToLower();
		if (Name == TEXT("characterselect"))
		{
			Name = TEXT("character select");
		}
		return Name;
	}

	FMapPaths MakeMapPaths(const FUianaImportConfig& Config)
	{
		FMapPaths Paths;
		Paths.Root = FPaths::Combine(Config.MapsRoot(), Config.MapName);
		Paths.Actors = FPaths::Combine(Paths.Root, TEXT("actors"));
		Paths.Materials = FPaths::Combine(Paths.Root, TEXT("materials"));
		Paths.MaterialsOverride = FPaths::Combine(Paths.Root, TEXT("materials_ovr"));
		Paths.Objects = FPaths::Combine(Paths.Root, TEXT("objects"));
		Paths.UMaps = FPaths::Combine(Paths.Root, TEXT("umaps"));
		return Paths;
	}

	void EnsureDirectory(const FString& InPath)
	{
		IFileManager::Get().MakeDirectory(*InPath, true);
	}

	void EnsureMapDirectories(const FUianaImportConfig& Config, const FMapPaths& Paths)
	{
		EnsureDirectory(Config.ExportRoot);
		EnsureDirectory(Config.AssetsRoot());
		EnsureDirectory(Config.MapsRoot());
		EnsureDirectory(Paths.Root);
		EnsureDirectory(Paths.Actors);
		EnsureDirectory(Paths.Materials);
		EnsureDirectory(Paths.MaterialsOverride);
		EnsureDirectory(Paths.Objects);
		EnsureDirectory(Paths.UMaps);
	}

	bool RunExternalTool(const FString& ExecutablePath, const FString& Args)
	{
		if (!FPaths::FileExists(ExecutablePath))
		{
			UE_LOG(LogTemp, Error, TEXT("Uiana: missing tool: %s"), *ExecutablePath);
			return false;
		}

		FProcHandle Proc = FPlatformProcess::CreateProc(*ExecutablePath, *Args, true, false, false, nullptr, 0, nullptr, nullptr);
		if (!Proc.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Uiana: failed to launch tool: %s"), *ExecutablePath);
			return false;
		}

		FPlatformProcess::WaitForProc(Proc);
		int32 ReturnCode = 1;
		FPlatformProcess::GetProcReturnCode(Proc, &ReturnCode);
		FPlatformProcess::CloseProc(Proc);
		if (ReturnCode != 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Uiana: tool returned %d: %s %s"), ReturnCode, *ExecutablePath, *Args);
		}
		return true;
	}

	bool ReadJsonArray(const FString& FilePath, TArray<TSharedPtr<FJsonValue>>& OutArray)
	{
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *FilePath))
		{
			return false;
		}

		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		return FJsonSerializer::Deserialize(Reader, OutArray);
	}

	bool ReadJsonObject(const FString& FilePath, TSharedPtr<FJsonObject>& OutObject)
	{
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *FilePath))
		{
			return false;
		}

		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		return FJsonSerializer::Deserialize(Reader, OutObject);
	}

	void SaveJsonArray(const FString& FilePath, const TArray<TSharedPtr<FJsonValue>>& JsonArray)
	{
		FString OutJson;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
		FJsonSerializer::Serialize(JsonArray, Writer);
		FFileHelper::SaveStringToFile(OutJson, *FilePath);
	}

	void ListFiles(const FString& Dir, const FString& Extension, TArray<FString>& OutFiles)
	{
		OutFiles.Reset();
		IFileManager::Get().FindFiles(OutFiles, *FPaths::Combine(Dir, FString::Printf(TEXT("*.%s"), *Extension)), true, false);
		for (FString& Name : OutFiles)
		{
			Name = FPaths::Combine(Dir, Name);
		}
	}

	FString GetObjectPathFromRef(const TSharedPtr<FJsonObject>& Obj, const bool bIsMaterialRef)
	{
		if (!Obj.IsValid())
		{
			return FString();
		}

		if (bIsMaterialRef)
		{
			return Obj->GetStringField(TEXT("ObjectPath"));
		}

		const TSharedPtr<FJsonObject>* Props = nullptr;
		if (!Obj->TryGetObjectField(TEXT("Properties"), Props) || !Props || !(*Props).IsValid())
		{
			return FString();
		}

		const TSharedPtr<FJsonObject>* StaticMesh = nullptr;
		if (!(*Props)->TryGetObjectField(TEXT("StaticMesh"), StaticMesh) || !StaticMesh || !(*StaticMesh).IsValid())
		{
			return FString();
		}

		return (*StaticMesh)->GetStringField(TEXT("ObjectPath"));
	}

	FString ToBackslashObjectPath(FString InObjectPath)
	{
		int32 DotIndex = INDEX_NONE;
		if (InObjectPath.FindChar(TEXT('.'), DotIndex))
		{
			InObjectPath = InObjectPath.Left(DotIndex);
		}
		InObjectPath.ReplaceInline(TEXT("/"), TEXT("\\"));
		return InObjectPath;
	}

	FString ObjectNameFromPath(const FString& ObjectPath)
	{
		const FString Sanitized = ObjectPath.Replace(TEXT("'"), TEXT(""));
		FString Left;
		FString Right;
		if (Sanitized.Split(TEXT("."), &Left, &Right, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
		{
			return Right;
		}
		return FPaths::GetBaseFilename(Sanitized);
	}

	FString ObjectNameFromObjectNameField(const FString& ObjectNameField)
	{
		int32 SpaceIdx = INDEX_NONE;
		if (ObjectNameField.FindLastChar(TEXT(' '), SpaceIdx))
		{
			return ObjectNameField.Mid(SpaceIdx + 1);
		}
		return ObjectNameField;
	}

	FString PathConvert(const FString& InPath)
	{
		FString Out = InPath;
		Out.ReplaceInline(TEXT("ShooterGame\\Content"), TEXT("Game"));
		Out.ReplaceInline(TEXT("Engine\\Content"), TEXT("Engine"));
		return Out;
	}

	bool IsBlacklisted(const FString& Name)
	{
		static const TArray<FString> Blacklist = {
			TEXT("navmesh"), TEXT("_breakable"), TEXT("_collision"), TEXT("windstreaks_plane"), TEXT("sm_port_snowflakes_boundmesh"),
			TEXT("m_pitt_caustics_box"), TEXT("box_for_volumes"), TEXT("bombsitemarker_0_bombsitea_glow"),
			TEXT("bombsitemarker_0_bombsiteb_glow"), TEXT("_col"), TEXT("m_pitt_lamps_glow"), TEXT("sm_pitt_water_lid"),
			TEXT("dirtskirt"), TEXT("tech_0_rebelsupplycargotarpcollision")
		};

		const FString Lower = Name.ToLower();
		for (const FString& Blocked : Blacklist)
		{
			if (Lower.Contains(Blocked))
			{
				return true;
			}
		}
		return false;
	}

	bool ParseTransform(const TSharedPtr<FJsonObject>& Props, FTransform& OutTransform)
	{
		if (!Props.IsValid())
		{
			return false;
		}

		FVector Location = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;
		FVector Scale = FVector(1.0, 1.0, 1.0);
		bool bHasAny = false;

		const TSharedPtr<FJsonObject>* RelLoc = nullptr;
		if (Props->TryGetObjectField(TEXT("RelativeLocation"), RelLoc) && RelLoc && (*RelLoc).IsValid())
		{
			Location.X = (*RelLoc)->GetNumberField(TEXT("X"));
			Location.Y = (*RelLoc)->GetNumberField(TEXT("Y"));
			Location.Z = (*RelLoc)->GetNumberField(TEXT("Z"));
			bHasAny = true;
		}

		const TSharedPtr<FJsonObject>* RelRot = nullptr;
		if (Props->TryGetObjectField(TEXT("RelativeRotation"), RelRot) && RelRot && (*RelRot).IsValid())
		{
			Rotation.Roll = (*RelRot)->GetNumberField(TEXT("Roll"));
			Rotation.Pitch = (*RelRot)->GetNumberField(TEXT("Pitch"));
			Rotation.Yaw = (*RelRot)->GetNumberField(TEXT("Yaw"));
			bHasAny = true;
		}

		const TSharedPtr<FJsonObject>* RelScale = nullptr;
		if (Props->TryGetObjectField(TEXT("RelativeScale3D"), RelScale) && RelScale && (*RelScale).IsValid())
		{
			Scale.X = (*RelScale)->GetNumberField(TEXT("X"));
			Scale.Y = (*RelScale)->GetNumberField(TEXT("Y"));
			Scale.Z = (*RelScale)->GetNumberField(TEXT("Z"));
			bHasAny = true;
		}

		OutTransform = FTransform(Rotation, Location, Scale);
		return bHasAny;
	}

	UMaterialInterface* LoadMaterialFromRef(const TSharedPtr<FJsonObject>& MaterialRef)
	{
		if (!MaterialRef.IsValid())
		{
			return nullptr;
		}

		const FString ObjectName = ObjectNameFromObjectNameField(MaterialRef->GetStringField(TEXT("ObjectName")));
		if (ObjectName.Contains(TEXT("MaterialInstanceDynamic")))
		{
			return nullptr;
		}

		const FString MatName = (ObjectName == TEXT("Stone_M2_Steps_MI1")) ? TEXT("Stone_M2_Steps_MI") : ObjectName;
		const FString AssetPath = FString::Printf(TEXT("/Game/ValorantContent/Materials/%s.%s"), *MatName, *MatName);
		return Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *AssetPath));
	}

	void SaveUniqueList(const FString& OutPath, const TArray<FString>& InLines)
	{
		TSet<FString> Unique;
		FString OutContent;
		for (const FString& Item : InLines)
		{
			if (Item.IsEmpty() || Unique.Contains(Item))
			{
				continue;
			}
			Unique.Add(Item);
			OutContent += Item;
			OutContent += LINE_TERMINATOR;
		}
		FFileHelper::SaveStringToFile(OutContent, *OutPath);
	}

	void AppendMaterialsFromStaticMeshDefinition(const TArray<TSharedPtr<FJsonValue>>& JsonArray, TArray<FString>& OutMaterials)
	{
		for (const TSharedPtr<FJsonValue>& Value : JsonArray)
		{
			const TSharedPtr<FJsonObject> Obj = Value->AsObject();
			if (!Obj.IsValid() || Obj->GetStringField(TEXT("Type")) != TEXT("StaticMesh"))
			{
				continue;
			}

			const TSharedPtr<FJsonObject>* Props = nullptr;
			if (!Obj->TryGetObjectField(TEXT("Properties"), Props) || !(*Props).IsValid())
			{
				continue;
			}

			const TArray<TSharedPtr<FJsonValue>>* StaticMaterials = nullptr;
			if (!(*Props)->TryGetArrayField(TEXT("StaticMaterials"), StaticMaterials) || !StaticMaterials)
			{
				continue;
			}

			for (const TSharedPtr<FJsonValue>& MatValue : *StaticMaterials)
			{
				const TSharedPtr<FJsonObject> MatObject = MatValue->AsObject();
				if (!MatObject.IsValid())
				{
					continue;
				}
				const TSharedPtr<FJsonObject>* MatInterface = nullptr;
				if (!MatObject->TryGetObjectField(TEXT("MaterialInterface"), MatInterface) || !(*MatInterface).IsValid())
				{
					continue;
				}
				OutMaterials.Add(ToBackslashObjectPath((*MatInterface)->GetStringField(TEXT("ObjectPath"))));
			}
		}
	}

	void FilterUMapAndCollectAssets(
		const TArray<TSharedPtr<FJsonValue>>& InObjects,
		TArray<TSharedPtr<FJsonValue>>& OutFiltered,
		TArray<FString>& OutObjects,
		TArray<FString>& OutOverrideMaterials,
		TArray<FString>& OutActors)
	{
		static const TSet<FString> MeshTypes = {
			TEXT("staticmesh"), TEXT("staticmeshcomponent"), TEXT("instancedstaticmeshcomponent"), TEXT("hierarchicalinstancedstaticmeshcomponent")
		};

		static const TSet<FString> GenericTypes = {
			TEXT("pointlightcomponent"), TEXT("postprocessvolume"), TEXT("culldistancevolume"), TEXT("scenecomponent"),
			TEXT("lightmasscharacterindirectdetailvolume"), TEXT("brushcomponent"), TEXT("precomputedvisibilityvolume"),
			TEXT("rectlightcomponent"), TEXT("spotlightcomponent"), TEXT("skylightcomponent"), TEXT("scenecapturecomponentcube"),
			TEXT("lightmassimportancevolume"), TEXT("billboardcomponent"), TEXT("directionallightcomponent"),
			TEXT("exponentialheightfogcomponent"), TEXT("lightmassportalcomponent"), TEXT("spherereflectioncapturecomponent")
		};

		for (const TSharedPtr<FJsonValue>& Value : InObjects)
		{
			const TSharedPtr<FJsonObject> Obj = Value->AsObject();
			if (!Obj.IsValid())
			{
				continue;
			}

			const FString Type = Obj->GetStringField(TEXT("Type"));
			const FString TypeLower = Type.ToLower();
			const bool bKeep = MeshTypes.Contains(TypeLower) || GenericTypes.Contains(TypeLower) || TypeLower == TEXT("decalcomponent") || TypeLower.EndsWith(TEXT("_c"));
			if (!bKeep)
			{
				continue;
			}

			OutFiltered.Add(Value);

			const TSharedPtr<FJsonObject>* Props = nullptr;
			const bool bHasProps = Obj->TryGetObjectField(TEXT("Properties"), Props) && Props && (*Props).IsValid();

			if (Type.EndsWith(TEXT("_C")) && Obj->HasField(TEXT("Template")))
			{
				OutActors.Add(Obj->GetStringField(TEXT("Template")));
			}

			if (!bHasProps)
			{
				continue;
			}

			const TSharedPtr<FJsonObject>* StaticMesh = nullptr;
			if ((*Props)->TryGetObjectField(TEXT("StaticMesh"), StaticMesh) && StaticMesh && (*StaticMesh).IsValid())
			{
				OutObjects.Add(ToBackslashObjectPath((*StaticMesh)->GetStringField(TEXT("ObjectPath"))));

				const TArray<TSharedPtr<FJsonValue>>* OverrideMaterials = nullptr;
				if ((*Props)->TryGetArrayField(TEXT("OverrideMaterials"), OverrideMaterials) && OverrideMaterials)
				{
					for (const TSharedPtr<FJsonValue>& MatValue : *OverrideMaterials)
					{
						const TSharedPtr<FJsonObject> MatObj = MatValue->AsObject();
						if (!MatObj.IsValid())
						{
							continue;
						}
						OutOverrideMaterials.Add(ToBackslashObjectPath(MatObj->GetStringField(TEXT("ObjectPath"))));
					}
				}
			}

			const TSharedPtr<FJsonObject>* DecalMaterial = nullptr;
			if ((*Props)->TryGetObjectField(TEXT("DecalMaterial"), DecalMaterial) && DecalMaterial && (*DecalMaterial).IsValid())
			{
				OutOverrideMaterials.Add(ToBackslashObjectPath((*DecalMaterial)->GetStringField(TEXT("ObjectPath"))));
			}
		}
	}

	FString ReadValorantVersion()
	{
		const FString VersionPath = TEXT("C:/ProgramData/Riot Games/Metadata/valorant.live/valorant.live.ok");
		if (!FPaths::FileExists(VersionPath))
		{
			return FString();
		}

		FString Text;
		if (!FFileHelper::LoadFileToString(Text, *VersionPath))
		{
			return FString();
		}

		FString FirstLine;
		Text.Split(TEXT("\n"), &FirstLine, nullptr);
		TArray<FString> SlashParts;
		FirstLine.ParseIntoArray(SlashParts, TEXT("/"), true);
		if (SlashParts.Num() == 0)
		{
			return FString();
		}
		TArray<FString> DotParts;
		SlashParts.Last().ParseIntoArray(DotParts, TEXT("."), true);
		return DotParts.Num() > 0 ? DotParts[0] : FString();
	}

	bool ShouldReExport(const FUianaImportConfig& Config, const FMapPaths& Paths)
	{
		const FString MarkerPath = FPaths::Combine(Paths.Root, TEXT("exported.yo"));
		if (!FPaths::FileExists(MarkerPath))
		{
			return true;
		}

		TArray<TSharedPtr<FJsonValue>> MarkerData;
		if (!ReadJsonArray(MarkerPath, MarkerData) || MarkerData.Num() == 0)
		{
			return true;
		}

		const FString PreviousVersion = MarkerData[0]->AsString();
		const FString CurrentVersion = ReadValorantVersion();
		return PreviousVersion != CurrentVersion;
	}

	void WriteExportMarker(const FMapPaths& Paths, const FUianaImportConfig& Config)
	{
		const FString Version = ReadValorantVersion();
		const FString JsonText = FString::Printf(TEXT("[\"%s\"]"), *Version);
		FFileHelper::SaveStringToFile(JsonText, *FPaths::Combine(Paths.Root, TEXT("exported.yo")));
		FFileHelper::SaveStringToFile(JsonText, *FPaths::Combine(Config.AssetsRoot(), TEXT("exported.yo")));
	}

	bool ExtractData(const FUianaImportConfig& Config, const FString& ExportDirectory, const FString& AssetListFile)
	{
		const FString Args = FString::Printf(
			TEXT("--game-directory \"%s\" --aes-key %s --export-directory \"%s\" --map-name \"%s\" --file-list \"%s\" --game-umaps \"%s\""),
			*Config.PaksRoot,
			*Config.AESKey,
			*ExportDirectory,
			*Config.MapName,
			*AssetListFile,
			*Config.UMapsListPath());
		return RunExternalTool(Config.Cue4ExtractorPath(), Args);
	}

	bool ExtractAssets(const FUianaImportConfig& Config, const FMapPaths& Paths)
	{
		const FString AssetObjects = FPaths::Combine(Paths.Root, TEXT("all_assets.txt"));
		const FString Args = FString::Printf(
			TEXT("--game-directory \"%s\" --aes-key %s --export-directory \"%s\" --file-list \"%s\" --game-umaps \"%s\" --export-assets"),
			*Config.PaksRoot,
			*Config.AESKey,
			*Config.AssetsRoot(),
			*AssetObjects,
			*Config.UMapsListPath());
		return RunExternalTool(Config.Cue4ExtractorPath(), Args);
	}

	FString BuildAssetFile(const FString& OutFilePath, const TArray<FString>& Items)
	{
		SaveUniqueList(OutFilePath, Items);
		return OutFilePath;
	}

	void ImportAllTexturesFromMaterialJson(const FString& MaterialFilePath, const FUianaImportConfig& Config, TSet<FString>& InOutTextures)
	{
		TArray<TSharedPtr<FJsonValue>> JsonArray;
		if (!ReadJsonArray(MaterialFilePath, JsonArray) || JsonArray.Num() == 0)
		{
			return;
		}

		const TSharedPtr<FJsonObject> RootObj = JsonArray[0]->AsObject();
		if (!RootObj.IsValid())
		{
			return;
		}

		const TSharedPtr<FJsonObject>* Props = nullptr;
		if (!RootObj->TryGetObjectField(TEXT("Properties"), Props) || !(*Props).IsValid())
		{
			return;
		}

		const TArray<TSharedPtr<FJsonValue>>* TextureParams = nullptr;
		if (!(*Props)->TryGetArrayField(TEXT("TextureParameterValues"), TextureParams) || !TextureParams)
		{
			return;
		}

		for (const TSharedPtr<FJsonValue>& ParamValue : *TextureParams)
		{
			const TSharedPtr<FJsonObject> ParamObj = ParamValue->AsObject();
			if (!ParamObj.IsValid())
			{
				continue;
			}

			const TSharedPtr<FJsonObject>* ParameterValueObj = nullptr;
			if (!ParamObj->TryGetObjectField(TEXT("ParameterValue"), ParameterValueObj) || !(*ParameterValueObj).IsValid())
			{
				continue;
			}

			const FString ObjectPath = (*ParameterValueObj)->GetStringField(TEXT("ObjectPath"));
			const FString NoExt = ObjectPath.LeftChop(4); // ".xxx"
			const FString Relative = PathConvert(NoExt.Replace(TEXT("/"), TEXT("\\"))).Replace(TEXT("'"), TEXT("")) + Config.TextureFormat;
			const FString LocalPath = FPaths::Combine(Config.AssetsRoot(), Relative);
			InOutTextures.Add(LocalPath);
		}
	}

	void ImportMaterialsFromJsonFolder(const FString& FolderPath)
	{
		TArray<FString> Files;
		ListFiles(FolderPath, TEXT("json"), Files);

		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();

		for (const FString& FilePath : Files)
		{
			TArray<TSharedPtr<FJsonValue>> JsonArray;
			if (!ReadJsonArray(FilePath, JsonArray) || JsonArray.Num() == 0)
			{
				continue;
			}

			const TSharedPtr<FJsonObject> RootObj = JsonArray[0]->AsObject();
			if (!RootObj.IsValid())
			{
				continue;
			}

			const FString MaterialName = RootObj->GetStringField(TEXT("Name"));
			const FString MaterialAssetPath = FString::Printf(TEXT("/Game/ValorantContent/Materials/%s.%s"), *MaterialName, *MaterialName);
			UMaterialInstanceConstant* MIC = Cast<UMaterialInstanceConstant>(StaticLoadObject(UMaterialInstanceConstant::StaticClass(), nullptr, *MaterialAssetPath));
			if (!MIC)
			{
				MIC = Cast<UMaterialInstanceConstant>(AssetTools.CreateAsset(
					MaterialName,
					TEXT("/Game/ValorantContent/Materials"),
					UMaterialInstanceConstant::StaticClass(),
					NewObject<UMaterialInstanceConstantFactoryNew>()));
			}
			if (!MIC)
			{
				continue;
			}

			const TSharedPtr<FJsonObject>* Props = nullptr;
			if (!RootObj->TryGetObjectField(TEXT("Properties"), Props) || !(*Props).IsValid())
			{
				continue;
			}

			FString ParentName = TEXT("BaseEnv_MAT_V4");
			const TSharedPtr<FJsonObject>* ParentObj = nullptr;
			if ((*Props)->TryGetObjectField(TEXT("Parent"), ParentObj) && ParentObj && (*ParentObj).IsValid())
			{
				ParentName = ObjectNameFromObjectNameField((*ParentObj)->GetStringField(TEXT("ObjectName")));
			}
			const FString ParentPath = FString::Printf(TEXT("/Uiana/Materials/%s.%s"), *ParentName, *ParentName);
			if (UMaterialInterface* ParentMat = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *ParentPath)))
			{
				MIC->SetParentEditorOnly(ParentMat);
			}

			const TArray<TSharedPtr<FJsonValue>>* ScalarParams = nullptr;
			if ((*Props)->TryGetArrayField(TEXT("ScalarParameterValues"), ScalarParams) && ScalarParams)
			{
				for (const TSharedPtr<FJsonValue>& Entry : *ScalarParams)
				{
					const TSharedPtr<FJsonObject> Obj = Entry->AsObject();
					if (!Obj.IsValid())
					{
						continue;
					}
					const TSharedPtr<FJsonObject>* ParamInfo = nullptr;
					if (!Obj->TryGetObjectField(TEXT("ParameterInfo"), ParamInfo) || !(*ParamInfo).IsValid())
					{
						continue;
					}
					const FName ParamName((*ParamInfo)->GetStringField(TEXT("Name")).ToLower());
					UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(MIC, ParamName, Obj->GetNumberField(TEXT("ParameterValue")));
				}
			}

			const TArray<TSharedPtr<FJsonValue>>* VectorParams = nullptr;
			if ((*Props)->TryGetArrayField(TEXT("VectorParameterValues"), VectorParams) && VectorParams)
			{
				for (const TSharedPtr<FJsonValue>& Entry : *VectorParams)
				{
					const TSharedPtr<FJsonObject> Obj = Entry->AsObject();
					if (!Obj.IsValid())
					{
						continue;
					}
					const TSharedPtr<FJsonObject>* ParamInfo = nullptr;
					const TSharedPtr<FJsonObject>* ParamValue = nullptr;
					if (!Obj->TryGetObjectField(TEXT("ParameterInfo"), ParamInfo) || !(*ParamInfo).IsValid() || !Obj->TryGetObjectField(TEXT("ParameterValue"), ParamValue) || !(*ParamValue).IsValid())
					{
						continue;
					}
					const FName ParamName((*ParamInfo)->GetStringField(TEXT("Name")).ToLower());
					const FLinearColor Color(
						(*ParamValue)->GetNumberField(TEXT("R")),
						(*ParamValue)->GetNumberField(TEXT("G")),
						(*ParamValue)->GetNumberField(TEXT("B")),
						(*ParamValue)->GetNumberField(TEXT("A")));
					UMaterialEditingLibrary::SetMaterialInstanceVectorParameterValue(MIC, ParamName, Color);
				}
			}

			const TArray<TSharedPtr<FJsonValue>>* TextureParams = nullptr;
			if ((*Props)->TryGetArrayField(TEXT("TextureParameterValues"), TextureParams) && TextureParams)
			{
				for (const TSharedPtr<FJsonValue>& Entry : *TextureParams)
				{
					const TSharedPtr<FJsonObject> Obj = Entry->AsObject();
					if (!Obj.IsValid())
					{
						continue;
					}
					const TSharedPtr<FJsonObject>* ParamInfo = nullptr;
					const TSharedPtr<FJsonObject>* ParamValue = nullptr;
					if (!Obj->TryGetObjectField(TEXT("ParameterInfo"), ParamInfo) || !(*ParamInfo).IsValid() || !Obj->TryGetObjectField(TEXT("ParameterValue"), ParamValue) || !(*ParamValue).IsValid())
					{
						continue;
					}

					const FString TextureName = ObjectNameFromPath((*ParamValue)->GetStringField(TEXT("ObjectPath")));
					const FString TexturePath = FString::Printf(TEXT("/Game/ValorantContent/Textures/%s.%s"), *TextureName, *TextureName);
					if (UTexture* Texture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), nullptr, *TexturePath)))
					{
						const FName ParamName((*ParamInfo)->GetStringField(TEXT("Name")).ToLower());
						UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(MIC, ParamName, Texture);
					}
				}
			}

			UMaterialEditingLibrary::UpdateMaterialInstance(MIC);

			UPackage* Package = MIC->GetOutermost();
			FSavePackageArgs SaveArgs;
			const FString Filename = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
			UPackage::SavePackage(Package, MIC, *Filename, SaveArgs);
		}
	}

	void ImportMapActors(const FString& UMapJsonPath, const FUianaImportConfig& Config)
	{
		UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
		if (!World)
		{
			return;
		}

		TArray<TSharedPtr<FJsonValue>> JsonArray;
		if (!ReadJsonArray(UMapJsonPath, JsonArray))
		{
			return;
		}

		for (const TSharedPtr<FJsonValue>& Value : JsonArray)
		{
			const TSharedPtr<FJsonObject> Obj = Value->AsObject();
			if (!Obj.IsValid())
			{
				continue;
			}

			const FString Type = Obj->GetStringField(TEXT("Type"));
			const TSharedPtr<FJsonObject>* Props = nullptr;
			if (!Obj->TryGetObjectField(TEXT("Properties"), Props) || !(*Props).IsValid())
			{
				continue;
			}

			const FString OuterName = Obj->GetStringField(TEXT("Outer"));
			FTransform Transform;
			if (!ParseTransform(*Props, Transform))
			{
				Transform = FTransform::Identity;
			}

			if (Config.bImportMeshes && (Type == TEXT("StaticMeshComponent") || Type == TEXT("InstancedStaticMeshComponent") || Type == TEXT("HierarchicalInstancedStaticMeshComponent")))
			{
				const TSharedPtr<FJsonObject>* StaticMesh = nullptr;
				if (!(*Props)->TryGetObjectField(TEXT("StaticMesh"), StaticMesh) || !(*StaticMesh).IsValid())
				{
					continue;
				}

				const FString MeshName = ObjectNameFromPath((*StaticMesh)->GetStringField(TEXT("ObjectPath")));
				const FString MeshAssetPath = FString::Printf(TEXT("/Game/ValorantContent/Meshes/%s.%s"), *MeshName, *MeshName);
				UStaticMesh* LoadedMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *MeshAssetPath));
				if (!LoadedMesh)
				{
					continue;
				}

				if (Type.Contains(TEXT("Instanced")))
				{
					AHismActor* Actor = World->SpawnActor<AHismActor>(AHismActor::StaticClass(), Transform);
					if (!Actor || !Actor->HismComponent)
					{
						continue;
					}
					Actor->SetActorLabel(OuterName);
					Actor->SetFolderPath(TEXT("Meshes/Instanced"));
					Actor->HismComponent->SetStaticMesh(LoadedMesh);

					const TArray<TSharedPtr<FJsonValue>>* Instances = nullptr;
					if (Obj->TryGetArrayField(TEXT("PerInstanceSMData"), Instances) && Instances)
					{
						for (const TSharedPtr<FJsonValue>& InstanceVal : *Instances)
						{
							const TSharedPtr<FJsonObject> InstanceObj = InstanceVal->AsObject();
							const TSharedPtr<FJsonObject>* TransformData = nullptr;
							if (!InstanceObj.IsValid() || !InstanceObj->TryGetObjectField(TEXT("TransformData"), TransformData) || !(*TransformData).IsValid())
							{
								continue;
							}

							const TSharedPtr<FJsonObject>* Translation = nullptr;
							const TSharedPtr<FJsonObject>* Scale3D = nullptr;
							const TSharedPtr<FJsonObject>* RotationQuat = nullptr;
							if (!(*TransformData)->TryGetObjectField(TEXT("Translation"), Translation) ||
								!(*TransformData)->TryGetObjectField(TEXT("Scale3D"), Scale3D) ||
								!(*TransformData)->TryGetObjectField(TEXT("Rotation"), RotationQuat))
							{
								continue;
							}

							const FVector Loc((*Translation)->GetNumberField(TEXT("X")), (*Translation)->GetNumberField(TEXT("Y")), (*Translation)->GetNumberField(TEXT("Z")));
							const FVector Scale((*Scale3D)->GetNumberField(TEXT("X")), (*Scale3D)->GetNumberField(TEXT("Y")), (*Scale3D)->GetNumberField(TEXT("Z")));
							const FQuat Quat(
								(*RotationQuat)->GetNumberField(TEXT("X")),
								(*RotationQuat)->GetNumberField(TEXT("Y")),
								(*RotationQuat)->GetNumberField(TEXT("Z")),
								(*RotationQuat)->GetNumberField(TEXT("W")));

							Actor->HismComponent->AddInstance(FTransform(Quat, Loc, Scale));
						}
					}
				}
				else
				{
					AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Transform);
					if (!Actor || !Actor->GetStaticMeshComponent())
					{
						continue;
					}
					Actor->SetActorLabel(OuterName);
					Actor->SetFolderPath(UMapJsonPath.Contains(TEXT("_VFX")) ? TEXT("Meshes/VFX") : TEXT("Meshes/Static"));
					Actor->GetStaticMeshComponent()->SetStaticMesh(LoadedMesh);

					const TArray<TSharedPtr<FJsonValue>>* OverrideMaterials = nullptr;
					if (Config.bImportMaterials && (*Props)->TryGetArrayField(TEXT("OverrideMaterials"), OverrideMaterials) && OverrideMaterials)
					{
						TArray<UMaterialInterface*> Materials;
						for (const TSharedPtr<FJsonValue>& MatValue : *OverrideMaterials)
						{
							Materials.Add(LoadMaterialFromRef(MatValue->AsObject()));
						}
						Actor->GetStaticMeshComponent()->SetOverrideMaterials(Materials);
					}
				}
				continue;
			}

			if (Config.bImportDecals && Type == TEXT("DecalComponent"))
			{
				ADecalActor* Decal = World->SpawnActor<ADecalActor>(ADecalActor::StaticClass(), Transform);
				if (!Decal || !Decal->GetDecal())
				{
					continue;
				}
				Decal->SetActorLabel(OuterName);
				Decal->SetFolderPath(TEXT("Decals"));

				const TSharedPtr<FJsonObject>* DecalMaterial = nullptr;
				if ((*Props)->TryGetObjectField(TEXT("DecalMaterial"), DecalMaterial) && DecalMaterial && (*DecalMaterial).IsValid())
				{
					if (UMaterialInterface* Mat = LoadMaterialFromRef(*DecalMaterial))
					{
						Decal->GetDecal()->SetDecalMaterial(Mat);
					}
				}
				continue;
			}

			if (Config.bImportLights && Type.EndsWith(TEXT("LightComponent")))
			{
				AActor* SpawnedLight = nullptr;
				if (Type == TEXT("PointLightComponent"))
				{
					SpawnedLight = World->SpawnActor<APointLight>(APointLight::StaticClass(), Transform);
				}
				else if (Type == TEXT("SpotLightComponent"))
				{
					SpawnedLight = World->SpawnActor<ASpotLight>(ASpotLight::StaticClass(), Transform);
				}
				else if (Type == TEXT("RectLightComponent"))
				{
					SpawnedLight = World->SpawnActor<ARectLight>(ARectLight::StaticClass(), Transform);
				}
				else if (Type == TEXT("SkyLightComponent"))
				{
					SpawnedLight = World->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), Transform);
				}
				else if (Type == TEXT("DirectionalLightComponent"))
				{
					SpawnedLight = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), Transform);
				}

				if (SpawnedLight)
				{
					SpawnedLight->SetActorLabel(OuterName);
					SpawnedLight->SetFolderPath(TEXT("Lights"));
				}
				continue;
			}

			if (Config.bImportBlueprints && Type.EndsWith(TEXT("_C")))
			{
				const FString BPName = Type.LeftChop(2);
				const FString BPPath = FString::Printf(TEXT("/Game/ValorantContent/Blueprints/%s.%s"), *BPName, *BPName);
				UBlueprint* BPAsset = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BPPath));
				if (!BPAsset || !BPAsset->GeneratedClass)
				{
					continue;
				}

				AActor* BPActor = World->SpawnActor<AActor>(BPAsset->GeneratedClass, Transform);
				if (BPActor)
				{
					BPActor->SetActorLabel(Obj->GetStringField(TEXT("Name")));
				}
			}
		}
	}

	bool PortedImportMap(const FUianaImportConfig& Config)
	{
		const FMapPaths Paths = MakeMapPaths(Config);
		EnsureMapDirectories(Config, Paths);

		TArray<FString> UMapJsonPaths;

		if (ShouldReExport(Config, Paths))
		{
			ExtractData(Config, Paths.UMaps, FString());
			ExtractAssets(Config, Paths);

			ListFiles(Paths.UMaps, TEXT("json"), UMapJsonPaths);
			TArray<FString> ObjectList;
			TArray<FString> ActorList;
			TArray<FString> OverrideMaterialList;

			for (const FString& UMapFile : UMapJsonPaths)
			{
				TArray<TSharedPtr<FJsonValue>> UMapArray;
				if (!ReadJsonArray(UMapFile, UMapArray))
				{
					continue;
				}

				TArray<TSharedPtr<FJsonValue>> Filtered;
				FilterUMapAndCollectAssets(UMapArray, Filtered, ObjectList, OverrideMaterialList, ActorList);
				SaveJsonArray(UMapFile, Filtered);
			}

			const FString ActorsAssetList = BuildAssetFile(FPaths::Combine(Paths.Root, TEXT("_assets_actors.txt")), ActorList);
			ExtractData(Config, Paths.Actors, ActorsAssetList);

			TArray<FString> ActorJsonFiles;
			ListFiles(Paths.Actors, TEXT("json"), ActorJsonFiles);
			for (const FString& ActorJson : ActorJsonFiles)
			{
				TArray<TSharedPtr<FJsonValue>> ActorArray;
				if (!ReadJsonArray(ActorJson, ActorArray))
				{
					continue;
				}
				TArray<FString> DummyActors;
				TArray<TSharedPtr<FJsonValue>> DummyFiltered;
				FilterUMapAndCollectAssets(ActorArray, DummyFiltered, ObjectList, OverrideMaterialList, DummyActors);
			}

			const FString ObjectsAssetList = BuildAssetFile(FPaths::Combine(Paths.Root, TEXT("_assets_objects.txt")), ObjectList);
			const FString OverrideMatsAssetList = BuildAssetFile(FPaths::Combine(Paths.Root, TEXT("_assets_materials_ovr.txt")), OverrideMaterialList);
			ExtractData(Config, Paths.Objects, ObjectsAssetList);
			ExtractData(Config, Paths.MaterialsOverride, OverrideMatsAssetList);

			TArray<FString> ObjectJsonFiles;
			ListFiles(Paths.Objects, TEXT("json"), ObjectJsonFiles);
			TArray<FString> MaterialsList;
			for (const FString& ObjFile : ObjectJsonFiles)
			{
				TArray<TSharedPtr<FJsonValue>> ObjArray;
				if (ReadJsonArray(ObjFile, ObjArray))
				{
					AppendMaterialsFromStaticMeshDefinition(ObjArray, MaterialsList);
				}
			}

			TArray<FString> AllAssets;
			AllAssets.Reserve(ObjectList.Num() + MaterialsList.Num() + OverrideMaterialList.Num());
			for (const FString& Path : ObjectList)
			{
				AllAssets.Add(PathConvert(Path));
			}
			for (const FString& Path : MaterialsList)
			{
				AllAssets.Add(PathConvert(Path));
			}
			for (const FString& Path : OverrideMaterialList)
			{
				AllAssets.Add(PathConvert(Path));
			}
			BuildAssetFile(FPaths::Combine(Paths.Root, TEXT("all_assets.txt")), AllAssets);

			const FString MaterialAssetList = BuildAssetFile(FPaths::Combine(Paths.Root, TEXT("_assets_materials.txt")), MaterialsList);
			ExtractData(Config, Paths.Materials, MaterialAssetList);
			ExtractAssets(Config, Paths);
			WriteExportMarker(Paths, Config);
		}
		else
		{
			ListFiles(Paths.UMaps, TEXT("json"), UMapJsonPaths);
		}

		if (Config.bImportMaterials)
		{
			TSet<FString> AllTextures;
			TArray<FString> MatFiles;
			ListFiles(Paths.Materials, TEXT("json"), MatFiles);
			for (const FString& MatFile : MatFiles)
			{
				ImportAllTexturesFromMaterialJson(MatFile, Config, AllTextures);
			}
			TArray<FString> MatOvrFiles;
			ListFiles(Paths.MaterialsOverride, TEXT("json"), MatOvrFiles);
			for (const FString& MatFile : MatOvrFiles)
			{
				ImportAllTexturesFromMaterialJson(MatFile, Config, AllTextures);
			}

			if (AllTextures.Num() > 0)
			{
				TArray<FString> TextureArray = AllTextures.Array();
				UBPFL::ImportTextures(TextureArray);
			}
			ImportMaterialsFromJsonFolder(Paths.Materials);
			ImportMaterialsFromJsonFolder(Paths.MaterialsOverride);
		}

		if (Config.bImportMeshes)
		{
			TArray<FString> MeshSources;
			TArray<FString> Lines;
			FFileHelper::LoadFileToStringArray(Lines, *FPaths::Combine(Paths.Root, TEXT("_assets_objects.txt")));
			for (const FString& Line : Lines)
			{
				if (Line.IsEmpty())
				{
					continue;
				}
				const FString Lower = Line.ToLower();
				if (Lower.StartsWith(TEXT("engine\\")))
				{
					continue;
				}
				const FString Filename = FPaths::GetBaseFilename(Line);
				if (IsBlacklisted(Filename))
				{
					continue;
				}

				FString Relative = Line;
				Relative.ReplaceInline(TEXT("ShooterGame\\Content\\"), TEXT("Game\\"));
				Relative.ReplaceInline(TEXT("Engine\\Content\\"), TEXT("Engine\\"));
				const FString FullPath = FPaths::Combine(Config.AssetsRoot(), Relative + TEXT(".uemodel"));
				if (FPaths::FileExists(FullPath))
				{
					MeshSources.AddUnique(FullPath);
				}
			}

			if (MeshSources.Num() > 0)
			{
				UBPFL::ImportMeshes(MeshSources, Paths.Objects);
			}
		}

		ListFiles(Paths.UMaps, TEXT("json"), UMapJsonPaths);
		FScopedSlowTask SpawnTask(UMapJsonPaths.Num(), FText::FromString(TEXT("Importing levels")));
		SpawnTask.MakeDialog(true);
		for (int32 Idx = UMapJsonPaths.Num() - 1; Idx >= 0; --Idx)
		{
			const FString& UMapFile = UMapJsonPaths[Idx];
			SpawnTask.EnterProgressFrame(1.f, FText::FromString(FString::Printf(TEXT("Importing %d/%d"), UMapJsonPaths.Num() - Idx, UMapJsonPaths.Num())));
			ImportMapActors(UMapFile, Config);
		}

		return true;
	}
}

bool FUianaImporter::BuildConfig(const UUianaDataSettings& Settings, FUianaImportConfig& OutConfig)
{
	const UEnum* MapEnum = StaticEnum<WeaponRole>();
	if (!MapEnum)
	{
		return false;
	}

	OutConfig.MapName = NormalizeMapName(MapEnum->GetNameStringByValue(Settings.Map.GetValue()));
	OutConfig.AESKey = GDefaultAES;
	OutConfig.TextureFormat = TEXT(".png");
	OutConfig.ExportRoot = Settings.ExportFolder.Path;
	OutConfig.PaksRoot = Settings.PaksFolder.Path;
	OutConfig.bImportDecals = Settings.ImportDecals;
	OutConfig.bImportBlueprints = Settings.ImportBlueprints;
	OutConfig.bImportLights = Settings.ImportLights;
	OutConfig.bImportMeshes = Settings.ImportMeshes;
	OutConfig.bImportMaterials = Settings.ImportMaterials;
	OutConfig.bImportSublevels = Settings.UseSubLevels;
	OutConfig.LightmapResolutionMultiplier = FMath::Max(Settings.LightmapResolutionMultiplier, 0.01f);

	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("Uiana"));
	if (!Plugin.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Uiana: failed to resolve plugin root path."));
		return false;
	}

	const FString ContentRoot = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Content"));
	const FString PreferredRoot = FPaths::Combine(ContentRoot, TEXT("Importer"));
	if (IFileManager::Get().DirectoryExists(*PreferredRoot))
	{
		OutConfig.PluginRoot = PreferredRoot;
		return true;
	}

	TArray<FString> CandidateDirs;
	IFileManager::Get().FindFiles(CandidateDirs, *FPaths::Combine(ContentRoot, TEXT("*")), false, true);
	for (const FString& CandidateName : CandidateDirs)
	{
		const FString CandidateRoot = FPaths::Combine(ContentRoot, CandidateName);
		const FString CandidateMaps = FPaths::Combine(CandidateRoot, TEXT("assets"), TEXT("umaps.json"));
		const FString CandidateTools = FPaths::Combine(CandidateRoot, TEXT("tools"));
		if (FPaths::FileExists(CandidateMaps) && IFileManager::Get().DirectoryExists(*CandidateTools))
		{
			OutConfig.PluginRoot = CandidateRoot;
			return true;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("Uiana: failed to locate importer content root (expected Content/Importer or equivalent tools+assets folder)."));
	return false;
}

bool FUianaImporter::Run(const UUianaDataSettings& Settings)
{
	FUianaImportConfig Config;
	if (!BuildConfig(Settings, Config))
	{
		return false;
	}

	if (Config.ExportRoot.IsEmpty() || Config.PaksRoot.IsEmpty() || Config.MapName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Uiana: invalid settings. Export/Paks/Map must be set."));
		return false;
	}

	return PortedImportMap(Config);
}
