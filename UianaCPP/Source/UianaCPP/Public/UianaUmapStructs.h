#pragma once
#include "UianaMaterialStructs.h"
#include "UianaUmapStructs.generated.h"

USTRUCT()
struct FUianaLodDatumParam {
    GENERATED_BODY();
    UPROPERTY()
    FString MapBuildDataId;
};

USTRUCT()
struct FUianaRotationParam {
    GENERATED_BODY();
    UPROPERTY()
    double Pitch;
    UPROPERTY()
    double Yaw;
    UPROPERTY()
    double Roll;
};

USTRUCT()
struct FUianaPerInstanceSmDatum {
    GENERATED_BODY();
    UPROPERTY()
    FUiana3DVectorParam OffsetLocation;
    UPROPERTY()
    FUianaRotationParam RelativeRotation;
    UPROPERTY()
    FUiana3DVectorParam RelativeScale3D;
};

USTRUCT()
struct FUianaResponse {
    GENERATED_BODY();
    UPROPERTY()
    FString Channel;
    UPROPERTY()
    FString Response;
};

USTRUCT()
struct FUianaCollisionResponses {
    GENERATED_BODY();
    UPROPERTY()
    TArray<FUianaResponse> ResponseArray;
};

USTRUCT()
struct FUianaWalkableSlopeOverride {
    GENERATED_BODY();
    UPROPERTY()
    FString WalkableSlopeBehavior;
};

USTRUCT()
struct FUianaBodyInstance {
    GENERATED_BODY();
    UPROPERTY()
    double MaxAngularVelocity;
    UPROPERTY()
    FString CollisionEnabled;
    UPROPERTY()
    FString CollisionProfileName;
    UPROPERTY()
    FUianaCollisionResponses CollisionResponses;
    UPROPERTY()
    FString ObjectType;
    UPROPERTY()
    double MassInKgOverride;
    UPROPERTY()
    bool bOverrideWalkableSlopeOnInstance;
    UPROPERTY()
    FUianaWalkableSlopeOverride WalkableSlopeOverride;
    UPROPERTY()
    FUianaObjectParam PhysMaterialOverride;
    UPROPERTY()
    bool bAutoWeld;
};

USTRUCT()
struct FUianaBuiltInstanceBounds {
    GENERATED_BODY();
    UPROPERTY()
    FUiana3DVectorParam Min;
    UPROPERTY()
    FUiana3DVectorParam Max;
    UPROPERTY()
    int IsValid;
};

USTRUCT()
struct FUianaCacheMeshExtendedBounds {
    GENERATED_BODY();
    UPROPERTY()
    FUiana3DVectorParam Origin;
    UPROPERTY()
    FUiana3DVectorParam BoxExtent;
    UPROPERTY()
    double SphereRadius;
};

USTRUCT()
struct FUianaUmapLightmassSettings {
    GENERATED_BODY();
    UPROPERTY()
    bool bUseEmissiveForStaticLighting;
    UPROPERTY()
    float EmissiveBoost;
    UPROPERTY()
    float DiffuseBoost;
    UPROPERTY()
    bool bUseVertexNormalForHemisphereGather;
    UPROPERTY()
    bool bUseTwoSidedLighting;
    UPROPERTY()
    float FullyOccludedSamplesFraction;
    UPROPERTY()
    bool bLightAsBackFace;
};

USTRUCT()
struct FUianaOnComponentOverlapInvocationList {
    GENERATED_BODY();
    UPROPERTY()
    FUianaObjectParam Object;
    UPROPERTY()
    FString FunctionName;
};

USTRUCT()
struct FUianaOnComponentOverlap {
    GENERATED_BODY();
    UPROPERTY()
    TArray<FUianaOnComponentOverlapInvocationList> InvocationList;
};


USTRUCT()
struct FUianaUmapSettings {
    GENERATED_BODY();
    UPROPERTY()
    bool bOverrideColorSaturation;
    UPROPERTY()
    bool bOverrideColorGainShadows;
    UPROPERTY()
    bool bOverrideBloomIntensity;
    UPROPERTY()
    bool bOverrideBloomThreshold;
    UPROPERTY()
    bool bOverrideAutoExposureMinBrightness;
    UPROPERTY()
    bool bOverrideAutoExposureMaxBrightness;
    UPROPERTY()
    bool bOverrideAutoExposureBias;
    UPROPERTY()
    bool bOverrideVignetteIntensity;
    UPROPERTY()
    bool bOverrideIndirectLightingColor;
    UPROPERTY()
    bool bOverrideIndirectLightingIntensity;
    UPROPERTY()
    bool bOverrideAresAdaptiveSharpenEnable;
    UPROPERTY()
    bool bOverrideAresClarityEnable;
    UPROPERTY()
    bool bOverrideIndirectLightingScaleCurve;
    UPROPERTY()
    int WhiteTemp;
    UPROPERTY()
    FUiana4DVectorParam ColorSaturationShadows;
    UPROPERTY()
    FUiana4DVectorParam ColorGainHighlights;
    UPROPERTY()
    double BloomIntensity;
    UPROPERTY()
    double BloomThreshold;
    UPROPERTY()
    bool bOverrideAutoExposureBiasBackup;
    UPROPERTY()
    float AutoExposureMinBrightness;
    UPROPERTY()
    float AutoExposureMaxBrightness;
    UPROPERTY()
    FLinearColor IndirectLightingColor;
    UPROPERTY()
    double IndirectLightingIntensity;
    UPROPERTY()
    FUianaObjectParam IndirectLightingScaleCurve;
};

USTRUCT()
struct FUianaStreamingTextureDatum {
    GENERATED_BODY();
    UPROPERTY()
    int PackedRelativeBox;
    UPROPERTY()
    int TextureLevelIndex;
    UPROPERTY()
    double TexelFactor;
};

USTRUCT()
struct FUianaStringTable {
    GENERATED_BODY();
    UPROPERTY()
    FString TableId;
    UPROPERTY()
    FString Key;
};

USTRUCT()
struct FUianaUcsModifiedProperty {
    GENERATED_BODY();
    UPROPERTY()
    FUianaObjectParam MemberParent;
    UPROPERTY()
    FString MemberName;
    UPROPERTY()
    FString MemberGuid;
};


USTRUCT()
struct FUianaUmapProperties {
    GENERATED_BODY();
    UPROPERTY()
    FUianaUmapSettings Settings;
    UPROPERTY()
    bool bUnbound;
    UPROPERTY()
    FString BrushType;
    UPROPERTY()
    FUianaObjectParam Brush;
    UPROPERTY()
    FUianaObjectParam BrushComponent;
    UPROPERTY()
    bool bHidden;
    UPROPERTY()
    FString SpawnCollisionHandlingMethod;
    UPROPERTY()
    FUianaObjectParam RootComponent;
    UPROPERTY()
    FUianaObjectParam BrushBodySetup;
    UPROPERTY()
    FUianaBodyInstance BodyInstance;
    UPROPERTY()
    FUiana3DVectorParam RelativeLocation;
    UPROPERTY()
    FUiana3DVectorParam RelativeScale3D;
    UPROPERTY()
    FUianaRotationParam RelativeRotation;
    UPROPERTY()
    FUianaObjectParam AttachParent;
    UPROPERTY()
    int FieldOfView;
    UPROPERTY()
    int AspectRatio;
    UPROPERTY()
    int UcsSerializationIndex;
    UPROPERTY()
    bool bNetAddressable;
    UPROPERTY()
    FString CreationMethod;
    UPROPERTY()
    TArray<FUianaUcsModifiedProperty> UcsModifiedProperties;
    UPROPERTY()
    FUianaObjectParam StaticMesh;
    UPROPERTY()
    bool bOverrideLightMapRes;
    UPROPERTY()
    bool bUseDefaultCollision;
    UPROPERTY()
    int OverriddenLightMapRes;
    UPROPERTY()
    TArray<FUianaStreamingTextureDatum> StreamingTextureData;
    UPROPERTY()
    FString UmbraCullingMode;
    UPROPERTY()
    bool CastShadow;
    UPROPERTY()
    FString CanCharacterStepUpOn;
    UPROPERTY()
    int VisibilityId;
    UPROPERTY()
    bool bDisallowMeshPaintPerInstance;
    UPROPERTY()
    FUianaUmapLightmassSettings LightmassSettings;
    UPROPERTY()
    TArray<FUianaObjectParam> OverrideMaterials;
    UPROPERTY()
    int ForcedLodModel;
    UPROPERTY()
    bool bVisibleInReflectionCaptures;
    UPROPERTY()
    bool bTreatAsBackgroundForOcclusion;
    UPROPERTY()
    double OuterConeAngle;
    UPROPERTY()
    int SourceRadius;
    UPROPERTY()
    int AttenuationRadius;
    UPROPERTY()
    FString LightGuid;
    UPROPERTY()
    int Intensity;
    UPROPERTY()
    FLinearColor LightColor;
    UPROPERTY()
    int IndirectLightingIntensity;
    UPROPERTY()
    FString Mobility;
    UPROPERTY()
    bool bUseInverseSquaredFalloff;
    UPROPERTY()
    double InnerConeAngle;
    UPROPERTY()
    bool CastShadows;
    UPROPERTY()
    FString SourceType;
    UPROPERTY()
    FUianaObjectParam Cubemap;
    UPROPERTY()
    int SkyDistanceThreshold;
    UPROPERTY()
    double BarnDoorAngle;
    UPROPERTY()
    FString IntensityUnits;
    UPROPERTY()
    int SourceHeight;
    UPROPERTY()
    double SourceWidth;
    UPROPERTY()
    double BarnDoorLength;
    UPROPERTY()
    FUianaObjectParam IesTexture;
    UPROPERTY()
    double SourceLength;
    UPROPERTY()
    int SoftSourceRadius;
    UPROPERTY()
    FUianaObjectParam PreviewBox;
    UPROPERTY()
    int InstancingRandomSeed;
    UPROPERTY()
    bool bGenerateOverlapEvents;
    UPROPERTY()
    bool bReceivesDecals;
    UPROPERTY()
    bool bHasPerInstanceHitProxies;
    UPROPERTY()
    double FogDensity;
    UPROPERTY()
    double FogHeightFalloff;
    UPROPERTY()
    FLinearColor FogInscatteringColor;
    UPROPERTY()
    int DirectionalInscatteringExponent;
    UPROPERTY()
    double FogMaxOpacity;
    UPROPERTY()
    int StartDistance;
    UPROPERTY()
    FLinearColor ModulatedShadowColor;
    UPROPERTY()
    bool bUsedAsAtmosphereSunLight;
    UPROPERTY()
    int LightProbeIntensity;
    UPROPERTY()
    FUianaObjectParam AreaClass;
    UPROPERTY()
    FUianaOnComponentOverlap OnComponentBeginOverlap;
    UPROPERTY()
    FUianaOnComponentOverlap OnComponentEndOverlap;
    UPROPERTY()
    bool bCanEverAffectNavigation;
    UPROPERTY()
    bool bAffectDynamicIndirectLighting;
    UPROPERTY()
    bool bAffectDistanceFieldLighting;
    UPROPERTY()
    bool bCastDynamicShadow;
    UPROPERTY()
    bool bCastStaticShadow;
    UPROPERTY()
    bool bRenderInTargetViewMode;
    UPROPERTY()
    TArray<FString> ComponentTags;
    UPROPERTY()
    FUianaObjectParam DecalMaterial;
    UPROPERTY()
    FUianaStringTable Text;
    UPROPERTY()
    FString DetailMode;
    UPROPERTY()
    FString PrepassCullMode;
    UPROPERTY()
    bool bUseAsOccluder;
    UPROPERTY()
    bool bTranslucencyPreDecal;
    UPROPERTY()
    int TranslucencySortPriority;
    UPROPERTY()
    TArray<int> SortedInstances;
    UPROPERTY()
    int NumBuiltInstances;
    UPROPERTY()
    FUianaBuiltInstanceBounds BuiltInstanceBounds;
    UPROPERTY()
    FUianaCacheMeshExtendedBounds CacheMeshExtendedBounds;
    UPROPERTY()
    int InstanceCountToRender;
    UPROPERTY()
    TArray<int> InstanceReorderTable;
    UPROPERTY()
    bool bOverrideIntensity;
    UPROPERTY()
    int LightIntensityOverride;
    UPROPERTY()
    bool bOverrideColor;
    UPROPERTY()
    FLinearColor LightColorOverride;
    // UPROPERTY()
    // TArray<nlohmann::json> AssetUserData;
    UPROPERTY()
    bool bReceivesFloorOnlyDecals;
    UPROPERTY()
    int SortOrder;
    UPROPERTY()
    FString LightColorType;
    UPROPERTY()
    double CachedVertexFogIntensityFromVolumes;
    UPROPERTY()
    bool bReceiveMobileCsmShadows;
    UPROPERTY()
    FUiana3DVectorParam DecalSize;
    UPROPERTY()
    int InfluenceRadius;
    UPROPERTY()
    FUianaObjectParam PreviewInfluenceRadius;
    // UPROPERTY()
    // nlohmann::json CaptureOffsetComponent;
    UPROPERTY()
    FString ReflectionSourceType;
    UPROPERTY()
    int SourceCubemapAngle;
    UPROPERTY()
    double Brightness;
    UPROPERTY()
    FString MapBuildDataId;
    UPROPERTY()
    int OrthoWidth;
    UPROPERTY()
    FUianaObjectParam TextureTarget;
    UPROPERTY()
    FString PrimitiveRenderMode;
};

USTRUCT()
struct FUianaUmapJson {
    GENERATED_BODY();
    UPROPERTY()
    FString Type;
    UPROPERTY()
    FString Name;
    UPROPERTY()
    FString Outer;
    UPROPERTY()
    FUianaUmapProperties Properties;
    FString Template;
    TArray<FUianaLodDatumParam> LodData;
    TArray<FUianaPerInstanceSmDatum> PerInstanceSmData;
};
