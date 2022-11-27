#pragma once

class UianaHelpers
{
public:
	enum ObjectType
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
	static void SaveJson(const TArray<TSharedPtr<FJsonValue>> json, const FString path);
	static void AddAllAssetPath(TArray<FString> &allAssets, const TArray<FString> assetsToAdd);
	static void AddPrefixPath(const FDirectoryPath path, TArray<FString> &suffixes);
	
	template <class PropType, class CppType>
	static bool SetStructProperty(void* data, const FProperty* objectProp, CppType value);
	template<class PropType, class CppType>
	static bool SetStructPropertiesFromJson(void* data, const FProperty* objectProp, const TSharedPtr<FJsonObject> jsonObj, const TArray<FName> jsonProps);
	template <class PropType>
    static bool SetActorProperty(UActorComponent* component, const FString propName, PropType propVal);

	static TEnumAsByte<EBlendMode> ParseBlendMode(const FString mode);
	static TEnumAsByte<EMaterialShadingModel> ParseShadingModel(const FString model);
	static TEnumAsByte<ECollisionTraceFlag> ParseCollisionTrace(const FString flag);
	static ObjectType ParseObjectType(const FString objType);

	static bool HasTransformComponent(const TSharedPtr<FJsonObject> comp);
	static void GetTransformComponent(const TSharedPtr<FJsonObject> comp, FTransform* transform);
	static void GetSceneTransformComponent(const TSharedPtr<FJsonObject> comp, FTransform* transform);
	static bool JsonObjContainsFields(const TSharedPtr<FJsonObject> obj, const TSet<FString> fields);
};
