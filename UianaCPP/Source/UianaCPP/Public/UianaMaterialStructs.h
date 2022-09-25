// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "UianaMaterialStructs.generated.h"

struct FVectorParameterValue;
struct FScalarParameterValue;
USTRUCT()
struct FUianaModelField
{
	GENERATED_BODY();
	UPROPERTY()
	FString ShadingModelField;
};

USTRUCT()
struct FUianaObjectParam
{
	GENERATED_BODY();
	UPROPERTY()
	FString ObjectName;
	UPROPERTY()
	FString ObjectPath;
};

USTRUCT()
struct FUiana3DVectorParam
{
	GENERATED_BODY();
	UPROPERTY()
	float X;
	UPROPERTY()
	float Y;
	UPROPERTY()
	float Z;
};

USTRUCT()
struct FUiana4DVectorParam : public FUiana3DVectorParam
{
	GENERATED_BODY();
	UPROPERTY()
	float W;
};

USTRUCT()
struct FUianaFunctionInfo
{
	GENERATED_BODY();
	UPROPERTY()
	FString StateId;
	UPROPERTY()
	FUianaObjectParam Function;
};

USTRUCT()
struct FUianaParameterInfo
{
	GENERATED_BODY();
	UPROPERTY()
	FString Name;
	UPROPERTY()
	FString Association;
	UPROPERTY()
	FString Index;
};

USTRUCT()
struct FUianaBaseParameterValue
{
	GENERATED_BODY();
	UPROPERTY()
	FMaterialParameterInfo ParameterInfo;
	UPROPERTY()
	FString ExpressionGUID;
};

USTRUCT()
struct FUianaTextureParameterValue : public FUianaBaseParameterValue
{
	GENERATED_BODY();
	UPROPERTY()
	FUianaObjectParam ParameterValue;
};

USTRUCT()
struct FUianaStaticSwitchParameterValue
{
	GENERATED_BODY();
	UPROPERTY()
	bool Value;
	UPROPERTY()
	bool bOverride;
};

USTRUCT()
struct FUianaStaticCompMaskParameterValue
{
	GENERATED_BODY();
	UPROPERTY()
	bool R;
	UPROPERTY()
	bool G;
	UPROPERTY()
	bool B;
	UPROPERTY()
	bool A;
	UPROPERTY()
	bool bOverride;
};

USTRUCT()
struct FUianaMaterialTextureParam
{
	GENERATED_BODY();
	UPROPERTY()
	int OutputIndex;
	UPROPERTY()
	FString InputName;
	UPROPERTY()
	FString ExpressionName;
	UPROPERTY()
	int8 Mask;
	UPROPERTY()
	int8 MaskR;
	UPROPERTY()
	int8 MaskG;
	UPROPERTY()
	int8 MaskB;
	UPROPERTY()
	int8 MaskA;
	UPROPERTY()
	bool UseConstant;
};

USTRUCT()
struct FUianaMaterialNormalParam : public FUianaMaterialTextureParam
{
	GENERATED_BODY();
	UPROPERTY()
	FUiana3DVectorParam Constant;
};

USTRUCT()
struct FUianaMaterialEmissiveParam : public FUianaMaterialTextureParam
{
	GENERATED_BODY();
	UPROPERTY()
	FLinearColor Constant;
};

USTRUCT()
struct FUianaMaterialPropertyParam : public FUianaMaterialTextureParam
{
	GENERATED_BODY();
	UPROPERTY()
	float Constant;
};

USTRUCT()
struct FUianaParameterEntry
{
	GENERATED_BODY();
	UPROPERTY()
	TArray<int> NameHashes;
	UPROPERTY()
	TArray<FUianaParameterInfo> ParameterInfos;
	UPROPERTY()
	TArray<FString> ExpressionGuids;
	UPROPERTY()
	TArray<bool> Overrides;
};

USTRUCT()
struct FUianaExpressionParameters
{
	GENERATED_BODY();
	UPROPERTY()
	TArray<FUianaParameterEntry> Entries;
	UPROPERTY()
	TArray<float> ScalarValues;
	UPROPERTY()
	TArray<FLinearColor> VectorValues;
	UPROPERTY()
	TArray<FUianaObjectParam> TextureValues;
};

USTRUCT()
struct FUianaMaterialExpressionData
{
	GENERATED_BODY();
	UPROPERTY()
	TArray<FUianaObjectParam> ReferencedTextures;
	UPROPERTY()
	TArray<FUianaFunctionInfo> FunctionInfos;
	UPROPERTY()
	TArray<bool> QualityLevelsUsed;
	UPROPERTY()
	FUianaExpressionParameters Parameters;
};

USTRUCT()
struct FUianaStaticParameters
{
	GENERATED_BODY();
	UPROPERTY()
	TArray<FUianaStaticSwitchParameterValue> StaticSwitchParameters;
	UPROPERTY()
	TArray<FUianaStaticCompMaskParameterValue> StaticComponentMaskParameters;
};

USTRUCT()
struct FUianaLightmassSettings
{
	GENERATED_BODY();
	UPROPERTY()
	bool bCastShadowAsTranslucent;
	UPROPERTY()
	bool bOverrideCastShadowAsTranslucent;
	UPROPERTY()
	bool bOverrideMaskedTranslucentOpacity;
	UPROPERTY()
	float MaskedTranslucentOpacity;
	UPROPERTY()
	float DiffuseBoost;
};

USTRUCT()
struct FUianaPropertyOverrides
{
	GENERATED_BODY();
	UPROPERTY()
	FString ShadingModel;
	UPROPERTY()
	FString CubemapMode;
	UPROPERTY()
	FString CubemapSource;
	UPROPERTY()
	FString BlendMode;
	UPROPERTY()
	int SortPriorityOffset;
	UPROPERTY()
	int IndirectLightingContributionValue;
	UPROPERTY()
	bool bOverride_Fresnel;
	UPROPERTY()
	bool bFresnel;
	UPROPERTY()
	bool bOverride_CubemapMode;
	UPROPERTY()
	bool bOverride_BlendMode;
	UPROPERTY()
	bool bOverride_TwoSided;
	UPROPERTY()
	bool bOverride_ShadingModel;
	UPROPERTY()
	bool bOverride_SortPriorityOffset;
	UPROPERTY()
	bool bOverride_VertexFog;
	UPROPERTY()
	bool bVertexFog;
	UPROPERTY()
	bool bOverride_CubemapSource;
	UPROPERTY()
	bool bOverride_IndirectLightingContributionValue;
	UPROPERTY()
	float OpacityMaskClipValue;
};

USTRUCT()
struct FUianaMaterialProperties
{
	GENERATED_BODY();
	UPROPERTY()
	bool bUsedAsSpecialEngineMaterial;
	UPROPERTY()
	bool bUsedWithInstancedStaticMeshes;
	UPROPERTY()
	bool bCanMaskedBeAssumedOpaque;
	UPROPERTY()
	bool bHasStaticPermutationResource;
	UPROPERTY()
	bool bUsedWithParticleSprites;
	UPROPERTY()
	bool bUsedWithMeshParticles;
	UPROPERTY()
	bool bUsedWithStaticLighting;
	UPROPERTY()
	bool bTangentSpaceNormal;
	UPROPERTY()
	bool bUsedWithSkeletalMesh;
	UPROPERTY()
	bool bVertexFog;
	UPROPERTY()
	FString StateId;
	UPROPERTY()
	FString ShadingModel;
	UPROPERTY()
	FString BlendMode;
	UPROPERTY()
	FString TranslucencyLightingMode;
	UPROPERTY()
	FString LightingSourceColor;
	UPROPERTY()
	int8 RefractionDepthBias;
	UPROPERTY()
	int8 IndirectLightingContributionValue;
	UPROPERTY()
	FUianaMaterialNormalParam Normal;
	UPROPERTY()
	FUianaMaterialEmissiveParam EmissiveColor;
	UPROPERTY()
	FUianaMaterialExpressionData CachedExpressionData;
	UPROPERTY()
	FUianaObjectParam PhysMaterial;
	UPROPERTY()
	FUianaObjectParam Parent;
	UPROPERTY()
	FUianaPropertyOverrides BasePropertyOverrides;
	UPROPERTY()
	TArray<FScalarParameterValue> ScalarParameterValues;
	UPROPERTY()
	TArray<FUianaTextureParameterValue> TextureParameterValues;
	UPROPERTY()
	TArray<FUianaObjectParam> CachedReferencedTextures;
	UPROPERTY()
	TArray<FMaterialTextureInfo> TextureStreamingData;
	UPROPERTY()
	TArray<FVectorParameterValue> VectorParameterValues;
	UPROPERTY()
	FUianaStaticParameters StaticParameters;
	UPROPERTY()
	FUianaLightmassSettings LightmassSettings;
	UPROPERTY()
	TArray<FUianaModelField> ShadingModels;
	UPROPERTY()
	FUianaMaterialNormalParam WorldPositionOffset;
	UPROPERTY()
	FUianaMaterialPropertyParam Metallic;
	UPROPERTY()
	FUianaMaterialPropertyParam Refraction;
};

USTRUCT()
struct FUianaMaterialJson
{
	GENERATED_BODY();
	UPROPERTY()
	FString Type;
	UPROPERTY()
	FString Name;
	UPROPERTY()
	FUianaMaterialProperties Properties;
}; 