// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef UIANA_UianaDataSettings_generated_h
#error "UianaDataSettings.generated.h already included, missing '#pragma once' in UianaDataSettings.h"
#endif
#define UIANA_UianaDataSettings_generated_h

#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h_24_SPARSE_DATA
#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h_24_RPC_WRAPPERS
#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h_24_RPC_WRAPPERS_NO_PURE_DECLS
#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h_24_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUUianaDataSettings(); \
	friend struct Z_Construct_UClass_UUianaDataSettings_Statics; \
public: \
	DECLARE_CLASS(UUianaDataSettings, UObject, COMPILED_IN_FLAGS(0 | CLASS_Transient | CLASS_DefaultConfig | CLASS_Config), CASTCLASS_None, TEXT("/Script/Uiana"), NO_API) \
	DECLARE_SERIALIZER(UUianaDataSettings) \
	static const TCHAR* StaticConfigName() {return TEXT("Engine");} \



#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h_24_INCLASS \
private: \
	static void StaticRegisterNativesUUianaDataSettings(); \
	friend struct Z_Construct_UClass_UUianaDataSettings_Statics; \
public: \
	DECLARE_CLASS(UUianaDataSettings, UObject, COMPILED_IN_FLAGS(0 | CLASS_Transient | CLASS_DefaultConfig | CLASS_Config), CASTCLASS_None, TEXT("/Script/Uiana"), NO_API) \
	DECLARE_SERIALIZER(UUianaDataSettings) \
	static const TCHAR* StaticConfigName() {return TEXT("Engine");} \



#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h_24_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UUianaDataSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UUianaDataSettings) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UUianaDataSettings); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UUianaDataSettings); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UUianaDataSettings(UUianaDataSettings&&); \
	NO_API UUianaDataSettings(const UUianaDataSettings&); \
public:


#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h_24_ENHANCED_CONSTRUCTORS \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UUianaDataSettings(UUianaDataSettings&&); \
	NO_API UUianaDataSettings(const UUianaDataSettings&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UUianaDataSettings); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UUianaDataSettings); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UUianaDataSettings)


#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h_20_PROLOG
#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h_24_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h_24_SPARSE_DATA \
	FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h_24_RPC_WRAPPERS \
	FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h_24_INCLASS \
	FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h_24_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h_24_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h_24_SPARSE_DATA \
	FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h_24_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h_24_INCLASS_NO_PURE_DECLS \
	FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h_24_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> UIANA_API UClass* StaticClass<class UUianaDataSettings>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_RangeRemake_Plugins_Uiana_Source_Uiana_Public_UianaDataSettings_h


#define FOREACH_ENUM_WEAPONROLE(op) \
	op(Ascent) \
	op(Split) \
	op(Bind) \
	op(Icebox) \
	op(Breeze) \
	op(Haven) \
	op(Fracture) \
	op(Range) \
	op(Pearl) \
	op(CharacterSelect) \
	op(Menu) 
PRAGMA_ENABLE_DEPRECATION_WARNINGS
