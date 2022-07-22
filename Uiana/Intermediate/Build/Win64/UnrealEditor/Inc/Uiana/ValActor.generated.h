// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
class UStaticMeshComponent;
class UBillboardComponent;
class UBoxComponent;
class UHierarchicalInstancedStaticMeshComponent;
class UCapsuleComponent;
#ifdef UIANA_ValActor_generated_h
#error "ValActor.generated.h already included, missing '#pragma once' in ValActor.h"
#endif
#define UIANA_ValActor_generated_h

#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h_13_SPARSE_DATA
#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h_13_RPC_WRAPPERS \
 \
	DECLARE_FUNCTION(execCreateBlockingVolumeComponent); \
	DECLARE_FUNCTION(execCreateBillboardComponent); \
	DECLARE_FUNCTION(execCreateBoxComponent); \
	DECLARE_FUNCTION(execCreateStaticComponent); \
	DECLARE_FUNCTION(execCreateInstanceComponent); \
	DECLARE_FUNCTION(execCreateCapsuleComponent);


#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h_13_RPC_WRAPPERS_NO_PURE_DECLS \
 \
	DECLARE_FUNCTION(execCreateBlockingVolumeComponent); \
	DECLARE_FUNCTION(execCreateBillboardComponent); \
	DECLARE_FUNCTION(execCreateBoxComponent); \
	DECLARE_FUNCTION(execCreateStaticComponent); \
	DECLARE_FUNCTION(execCreateInstanceComponent); \
	DECLARE_FUNCTION(execCreateCapsuleComponent);


#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h_13_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesAValActor(); \
	friend struct Z_Construct_UClass_AValActor_Statics; \
public: \
	DECLARE_CLASS(AValActor, AActor, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/Uiana"), NO_API) \
	DECLARE_SERIALIZER(AValActor)


#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h_13_INCLASS \
private: \
	static void StaticRegisterNativesAValActor(); \
	friend struct Z_Construct_UClass_AValActor_Statics; \
public: \
	DECLARE_CLASS(AValActor, AActor, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/Uiana"), NO_API) \
	DECLARE_SERIALIZER(AValActor)


#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h_13_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API AValActor(const FObjectInitializer& ObjectInitializer); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(AValActor) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, AValActor); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(AValActor); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API AValActor(AValActor&&); \
	NO_API AValActor(const AValActor&); \
public:


#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h_13_ENHANCED_CONSTRUCTORS \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API AValActor(AValActor&&); \
	NO_API AValActor(const AValActor&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, AValActor); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(AValActor); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(AValActor)


#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h_10_PROLOG
#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h_13_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h_13_SPARSE_DATA \
	FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h_13_RPC_WRAPPERS \
	FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h_13_INCLASS \
	FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h_13_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h_13_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h_13_SPARSE_DATA \
	FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h_13_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h_13_INCLASS_NO_PURE_DECLS \
	FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h_13_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> UIANA_API UClass* StaticClass<class AValActor>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_ValActor_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
