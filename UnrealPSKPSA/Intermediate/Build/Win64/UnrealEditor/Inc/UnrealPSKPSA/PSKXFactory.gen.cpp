// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "UnrealPSKPSA/Public/PSKXFactory.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodePSKXFactory() {}
// Cross Module References
	UNREALPSKPSA_API UClass* Z_Construct_UClass_UPSKXFactory_NoRegister();
	UNREALPSKPSA_API UClass* Z_Construct_UClass_UPSKXFactory();
	UNREALED_API UClass* Z_Construct_UClass_UFactory();
	UPackage* Z_Construct_UPackage__Script_UnrealPSKPSA();
// End Cross Module References
	void UPSKXFactory::StaticRegisterNativesUPSKXFactory()
	{
	}
	IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UPSKXFactory);
	UClass* Z_Construct_UClass_UPSKXFactory_NoRegister()
	{
		return UPSKXFactory::StaticClass();
	}
	struct Z_Construct_UClass_UPSKXFactory_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UECodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UPSKXFactory_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UFactory,
		(UObject* (*)())Z_Construct_UPackage__Script_UnrealPSKPSA,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UPSKXFactory_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "PSKXFactory.h" },
		{ "ModuleRelativePath", "Public/PSKXFactory.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_UPSKXFactory_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UPSKXFactory>::IsAbstract,
	};
	const UECodeGen_Private::FClassParams Z_Construct_UClass_UPSKXFactory_Statics::ClassParams = {
		&UPSKXFactory::StaticClass,
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
		METADATA_PARAMS(Z_Construct_UClass_UPSKXFactory_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_UPSKXFactory_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UPSKXFactory()
	{
		if (!Z_Registration_Info_UClass_UPSKXFactory.OuterSingleton)
		{
			UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UPSKXFactory.OuterSingleton, Z_Construct_UClass_UPSKXFactory_Statics::ClassParams);
		}
		return Z_Registration_Info_UClass_UPSKXFactory.OuterSingleton;
	}
	template<> UNREALPSKPSA_API UClass* StaticClass<UPSKXFactory>()
	{
		return UPSKXFactory::StaticClass();
	}
	DEFINE_VTABLE_PTR_HELPER_CTOR(UPSKXFactory);
	struct Z_CompiledInDeferFile_FID_ProjectBeka_Plugins_UnrealPSKPSA_Source_UnrealPSKPSA_Public_PSKXFactory_h_Statics
	{
		static const FClassRegisterCompiledInInfo ClassInfo[];
	};
	const FClassRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ProjectBeka_Plugins_UnrealPSKPSA_Source_UnrealPSKPSA_Public_PSKXFactory_h_Statics::ClassInfo[] = {
		{ Z_Construct_UClass_UPSKXFactory, UPSKXFactory::StaticClass, TEXT("UPSKXFactory"), &Z_Registration_Info_UClass_UPSKXFactory, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UPSKXFactory), 1974889660U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ProjectBeka_Plugins_UnrealPSKPSA_Source_UnrealPSKPSA_Public_PSKXFactory_h_4180021724(TEXT("/Script/UnrealPSKPSA"),
		Z_CompiledInDeferFile_FID_ProjectBeka_Plugins_UnrealPSKPSA_Source_UnrealPSKPSA_Public_PSKXFactory_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ProjectBeka_Plugins_UnrealPSKPSA_Source_UnrealPSKPSA_Public_PSKXFactory_h_Statics::ClassInfo),
		nullptr, 0,
		nullptr, 0);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
