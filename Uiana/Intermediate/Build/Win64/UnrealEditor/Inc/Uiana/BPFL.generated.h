// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
class UStaticMeshComponent;
struct FColor;
#ifdef UIANA_BPFL_generated_h
#error "BPFL.generated.h already included, missing '#pragma once' in BPFL.h"
#endif
#define UIANA_BPFL_generated_h

#define FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_15_SPARSE_DATA
#define FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_15_RPC_WRAPPERS \
 \
	DECLARE_FUNCTION(execPaintRandomSMVertices); \
	DECLARE_FUNCTION(execReturnFromHex); \
	DECLARE_FUNCTION(execPaintSMVertices);


#define FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_15_RPC_WRAPPERS_NO_PURE_DECLS \
 \
	DECLARE_FUNCTION(execPaintRandomSMVertices); \
	DECLARE_FUNCTION(execReturnFromHex); \
	DECLARE_FUNCTION(execPaintSMVertices);


#define FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_15_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUBPFL(); \
	friend struct Z_Construct_UClass_UBPFL_Statics; \
public: \
	DECLARE_CLASS(UBPFL, UBlueprintFunctionLibrary, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/Uiana"), NO_API) \
	DECLARE_SERIALIZER(UBPFL)


#define FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_15_INCLASS \
private: \
	static void StaticRegisterNativesUBPFL(); \
	friend struct Z_Construct_UClass_UBPFL_Statics; \
public: \
	DECLARE_CLASS(UBPFL, UBlueprintFunctionLibrary, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/Uiana"), NO_API) \
	DECLARE_SERIALIZER(UBPFL)


#define FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_15_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UBPFL(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UBPFL) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UBPFL); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UBPFL); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UBPFL(UBPFL&&); \
	NO_API UBPFL(const UBPFL&); \
public:


#define FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_15_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UBPFL(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UBPFL(UBPFL&&); \
	NO_API UBPFL(const UBPFL&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UBPFL); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UBPFL); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UBPFL)


#define FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_12_PROLOG
#define FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_15_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_15_SPARSE_DATA \
	FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_15_RPC_WRAPPERS \
	FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_15_INCLASS \
	FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_15_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_15_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_15_SPARSE_DATA \
	FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_15_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_15_INCLASS_NO_PURE_DECLS \
	FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_15_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> UIANA_API UClass* StaticClass<class UBPFL>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
