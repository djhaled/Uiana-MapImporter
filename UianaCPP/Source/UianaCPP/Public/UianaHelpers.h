#pragma once

class UianaHelpers
{
public:
	static void CreateFolder(FDirectoryPath &FolderPath, FString Root, FString Extension);
	static TSharedPtr<FJsonObject> ParseJson(FString InputStr);
	static void SaveJson(const TArray<TSharedPtr<FJsonValue>> json, const FString path);
	static void AddAllAssetPath(TArray<FString> &allAssets, const TArray<FString> assetsToAdd);
	static void AddPrefixPath(const FDirectoryPath path, TArray<FString> &suffixes);
	
	template <class PropType, class CppType>
	static bool SetStructProperty(void* data, const FProperty* objectProp, CppType value);
	template<class PropType, class CppType>
	static bool SetStructPropertiesFromJson(void* data, const FProperty* objectProp, const TSharedPtr<FJsonObject> jsonObj, const TArray<FName> jsonProps);

	static TEnumAsByte<EBlendMode> ParseBlendMode(const FString mode);
	static TEnumAsByte<EMaterialShadingModel> ParseShadingModel(const FString model);
};
