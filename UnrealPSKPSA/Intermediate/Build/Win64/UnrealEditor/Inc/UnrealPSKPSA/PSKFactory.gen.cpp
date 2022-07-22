// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "UnrealPSKPSA/Public/PSKFactory.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodePSKFactory() {}
// Cross Module References
	UNREALPSKPSA_API UClass* Z_Construct_UClass_UPSKFactory_NoRegister();
	UNREALPSKPSA_API UClass* Z_Construct_UClass_UPSKFactory();
	UNREALED_API UClass* Z_Construct_UClass_UFactory();
	UPackage* Z_Construct_UPackage__Script_UnrealPSKPSA();
// End Cross Module References
	void UPSKFactory::StaticRegisterNativesUPSKFactory()
	{
	}
	IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UPSKFactory);
	UClass* Z_Construct_UClass_UPSKFactory_NoRegister()
	{
		return UPSKFactory::StaticClass();
	}
	struct Z_Construct_UClass_UPSKFactory_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UECodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UPSKFactory_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UFactory,
		(UObject* (*)())Z_Construct_UPackage__Script_UnrealPSKPSA,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPSKFactory_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "PSKFactory.h" },
		{ "ModuleRelativePath", "Public/PSKFactory.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_UPSKFactory_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UPSKFactory>::IsAbstract,
	};
	const UECodeGen_Private::FClassParams Z_Construct_UClass_UPSKFactory_Statics::ClassParams = {
		&UPSKFactory::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		nullptr,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		0,
		0,
		0,
		0x001000A0u,
		METADATA_PARAMS(Z_Construct_UClass_UPSKFactory_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_UPSKFactory_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UPSKFactory()
	{
		if (!Z_Registration_Info_UClass_UPSKFactory.OuterSingleton)
		{
			UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UPSKFactory.OuterSingleton, Z_Construct_UClass_UPSKFactory_Statics::ClassParams);
		}
		return Z_Registration_Info_UClass_UPSKFactory.OuterSingleton;
	}
	template<> UNREALPSKPSA_API UClass* StaticClass<UPSKFactory>()
	{
		return UPSKFactory::StaticClass();
	}
	DEFINE_VTABLE_PTR_HELPER_CTOR(UPSKFactory);
	struct Z_CompiledInDeferFile_FID_RangeRemake_Plugins_UnrealPSKPSA_Source_UnrealPSKPSA_Public_PSKFactory_h_Statics
	{
		static const FClassRegisterCompiledInInfo ClassInfo[];
	};
	const FClassRegisterCompiledInInfo Z_CompiledInDeferFile_FID_RangeRemake_Plugins_UnrealPSKPSA_Source_UnrealPSKPSA_Public_PSKFactory_h_Statics::ClassInfo[] = {
		{ Z_Construct_UClass_UPSKFactory, UPSKFactory::StaticClass, TEXT("UPSKFactory"), &Z_Registration_Info_UClass_UPSKFactory, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UPSKFactory), 2626635841U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_RangeRemake_Plugins_UnrealPSKPSA_Source_UnrealPSKPSA_Public_PSKFactory_h_924007601(TEXT("/Script/UnrealPSKPSA"),
		Z_CompiledInDeferFile_FID_RangeRemake_Plugins_UnrealPSKPSA_Source_UnrealPSKPSA_Public_PSKFactory_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_RangeRemake_Plugins_UnrealPSKPSA_Source_UnrealPSKPSA_Public_PSKFactory_h_Statics::ClassInfo),
		nullptr, 0,
		nullptr, 0);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
