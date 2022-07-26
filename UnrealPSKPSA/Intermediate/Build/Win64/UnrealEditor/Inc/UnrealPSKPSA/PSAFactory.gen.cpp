// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "UnrealPSKPSA/Public/PSAFactory.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodePSAFactory() {}
// Cross Module References
	UNREALPSKPSA_API UClass* Z_Construct_UClass_UPSAFactory_NoRegister();
	UNREALPSKPSA_API UClass* Z_Construct_UClass_UPSAFactory();
	UNREALED_API UClass* Z_Construct_UClass_UFactory();
	UPackage* Z_Construct_UPackage__Script_UnrealPSKPSA();
// End Cross Module References
	void UPSAFactory::StaticRegisterNativesUPSAFactory()
	{
	}
	IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UPSAFactory);
	UClass* Z_Construct_UClass_UPSAFactory_NoRegister()
	{
		return UPSAFactory::StaticClass();
	}
	struct Z_Construct_UClass_UPSAFactory_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UECodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UPSAFactory_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UFactory,
		(UObject* (*)())Z_Construct_UPackage__Script_UnrealPSKPSA,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPSAFactory_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "PSAFactory.h" },
		{ "ModuleRelativePath", "Public/PSAFactory.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_UPSAFactory_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UPSAFactory>::IsAbstract,
	};
	const UECodeGen_Private::FClassParams Z_Construct_UClass_UPSAFactory_Statics::ClassParams = {
		&UPSAFactory::StaticClass,
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
		METADATA_PARAMS(Z_Construct_UClass_UPSAFactory_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_UPSAFactory_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UPSAFactory()
	{
		if (!Z_Registration_Info_UClass_UPSAFactory.OuterSingleton)
		{
			UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UPSAFactory.OuterSingleton, Z_Construct_UClass_UPSAFactory_Statics::ClassParams);
		}
		return Z_Registration_Info_UClass_UPSAFactory.OuterSingleton;
	}
	template<> UNREALPSKPSA_API UClass* StaticClass<UPSAFactory>()
	{
		return UPSAFactory::StaticClass();
	}
	DEFINE_VTABLE_PTR_HELPER_CTOR(UPSAFactory);
	struct Z_CompiledInDeferFile_FID_RangeRemake_Plugins_UnrealPSKPSA_Source_UnrealPSKPSA_Public_PSAFactory_h_Statics
	{
		static const FClassRegisterCompiledInInfo ClassInfo[];
	};
	const FClassRegisterCompiledInInfo Z_CompiledInDeferFile_FID_RangeRemake_Plugins_UnrealPSKPSA_Source_UnrealPSKPSA_Public_PSAFactory_h_Statics::ClassInfo[] = {
		{ Z_Construct_UClass_UPSAFactory, UPSAFactory::StaticClass, TEXT("UPSAFactory"), &Z_Registration_Info_UClass_UPSAFactory, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UPSAFactory), 3688621872U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_RangeRemake_Plugins_UnrealPSKPSA_Source_UnrealPSKPSA_Public_PSAFactory_h_714949131(TEXT("/Script/UnrealPSKPSA"),
		Z_CompiledInDeferFile_FID_RangeRemake_Plugins_UnrealPSKPSA_Source_UnrealPSKPSA_Public_PSAFactory_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_RangeRemake_Plugins_UnrealPSKPSA_Source_UnrealPSKPSA_Public_PSAFactory_h_Statics::ClassInfo),
		nullptr, 0,
		nullptr, 0);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
