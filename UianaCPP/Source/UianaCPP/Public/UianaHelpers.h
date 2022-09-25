#pragma once
#include<filesystem>

#include "UianaHelpers.generated.h"

USTRUCT()
struct FUMapObjectProperty
{
	GENERATED_BODY();
	UPROPERTY()
	FString ObjectName;
	UPROPERTY()
	FString ObjectPath;
};

USTRUCT()
struct FInfoProperty
{
	GENERATED_BODY();
	UPROPERTY()
	FString Name;
	UPROPERTY()
	FString Association;
	UPROPERTY()
	int Index;
};

USTRUCT()
struct FModelMaterialComponent
{
	GENERATED_BODY();
	UPROPERTY()
	FUMapObjectProperty MaterialInterface;
};

USTRUCT()
struct FParamComponent
{
	GENERATED_BODY()
	UPROPERTY()
	FInfoProperty ParameterInfo;
	UPROPERTY()
	FUMapObjectProperty ParameterValue;
	UPROPERTY()
	FString ExpressionGUID;
};

USTRUCT()
struct FModelProperties
{
	GENERATED_BODY();
	UPROPERTY()
	TArray<FModelMaterialComponent> StaticMaterials;
};

USTRUCT()
struct FMaterialProperties
{
	GENERATED_BODY()
	TOptional<FUMapObjectProperty> Parent;
	UPROPERTY()
	TArray<FParamComponent> TextureParameterValues;
	TOptional<TArray<FParamComponent>> VectorParameterValues;
	TOptional<FUMapObjectProperty> LightmassSetting;
	
};

USTRUCT()
struct FModelObject
{
	GENERATED_BODY();
	UPROPERTY()
	FString Type;
	UPROPERTY()
	FModelProperties Properties;
};

USTRUCT()
struct FMaterialObject
{
	GENERATED_BODY()
	UPROPERTY()
	FString Type;
	UPROPERTY()
	FString Name;
	UPROPERTY()
	FMaterialProperties Properties;
};
class UianaHelpers
{
public:
	static void CreateFolder(FDirectoryPath &FolderPath, FString Root, FString Extension);
	static TSharedPtr<FJsonObject> ParseJson(FString InputStr);
};
