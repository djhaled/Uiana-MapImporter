#pragma once
#include "Dom/JsonValue.h"

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

	const static TSet<FString> LightRelatedObjects, MeshRelatedObjects, DecalRelatedObjects, BlueprintRelatedObjects;
	
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
	static void DuplicateJsonObj(const TSharedPtr<FJsonObject>& Source, TSharedPtr<FJsonObject> &Dest);
	static void DuplicateJsonArray(const TArray<TSharedPtr<FJsonValue>>& Source, TArray<TSharedPtr<FJsonValue>>& Dest);
	static TSharedPtr<FJsonValue> DuplicateJsonValue(const TSharedPtr<FJsonValue>& Src);
};
