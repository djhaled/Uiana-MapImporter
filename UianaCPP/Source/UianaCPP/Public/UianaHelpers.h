#pragma once

class UianaHelpers
{
public:
	enum EObjectType
	{
		Mesh,
		Decal,
		Light,
		Blueprint,
		Unknown
	};

	const static inline TSet<FString> LightRelatedObjects = {"PointLightComponent", "PostProcessVolume", "PrecomputedVisibilityVolume", "CullDistanceVolume",
			  "RectLightComponent", "LightmassCharacterIndirectDetailVolume", "SpotLightComponent", "SkyLightComponent",
			  "LightmassImportanceVolume", "SceneCaptureComponentCube", "SphereReflectionCaptureComponent",
			  "DirectionalLightComponent", "ExponentialHeightFogComponent", "LightmassPortalComponent"};
	const static inline TSet<FString> MeshRelatedObjects = {"StaticMeshComponent", "InstancedStaticMeshComponent", "HierarchicalInstancedStaticMeshComponent"};
	const static inline TSet<FString> DecalRelatedObjects = {"DecalComponent"};
	const static inline TSet<FString> BlueprintRelatedObjects = {"SceneComponent"};
	
	static void CreateFolder(FDirectoryPath &FolderPath, FString Root, FString Extension);
	static TSharedPtr<FJsonObject> ParseJson(FString InputStr);
	static void SaveJson(const TArray<TSharedPtr<FJsonValue>> JsonObj, const FString Path);
	static void AddAllAssetPath(TArray<FString> &AllAssets, const TArray<FString> AssetsToAdd);
	static void AddPrefixPath(const FDirectoryPath Path, TArray<FString> &Suffixes);
	
	template <class PropType>
    static bool SetActorProperty(UClass* ActorClass, UObject* Component, const FString PropName, PropType PropVal);
	
	static TEnumAsByte<ECollisionTraceFlag> ParseCollisionTrace(const FString Flag);
	static EObjectType ParseObjectType(const FString ObjType);

	static bool HasTransformComponent(const TSharedPtr<FJsonObject> Comp);
	static FTransform GetTransformComponent(const TSharedPtr<FJsonObject> Comp);
	static FTransform GetSceneTransformComponent(const TSharedPtr<FJsonObject> Comp);
	static bool JsonObjContainsFields(const TSharedPtr<FJsonObject> Obj, const TSet<FString> Fields);
};
