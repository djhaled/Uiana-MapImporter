// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Uiana/Public/BPFL.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeBPFL() {}
// Cross Module References
	UIANA_API UClass* Z_Construct_UClass_UBPFL_NoRegister();
	UIANA_API UClass* Z_Construct_UClass_UBPFL();
	ENGINE_API UClass* Z_Construct_UClass_UBlueprintFunctionLibrary();
	UPackage* Z_Construct_UPackage__Script_Uiana();
	ENGINE_API UClass* Z_Construct_UClass_UStaticMeshComponent_NoRegister();
	COREUOBJECT_API UScriptStruct* Z_Construct_UScriptStruct_FColor();
// End Cross Module References
	DEFINE_FUNCTION(UBPFL::execPaintRandomSMVertices)
	{
		P_GET_OBJECT(UStaticMeshComponent,Z_Param_SMComp);
		P_FINISH;
		P_NATIVE_BEGIN;
		UBPFL::PaintRandomSMVertices(Z_Param_SMComp);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(UBPFL::execReturnFromHex)
	{
		P_GET_PROPERTY(FStrProperty,Z_Param_Beka);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(FColor*)Z_Param__Result=UBPFL::ReturnFromHex(Z_Param_Beka);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(UBPFL::execPaintSMVertices)
	{
		P_GET_OBJECT(UStaticMeshComponent,Z_Param_SMComp);
		P_GET_TARRAY(FColor,Z_Param_Bekalici);
		P_FINISH;
		P_NATIVE_BEGIN;
		UBPFL::PaintSMVertices(Z_Param_SMComp,Z_Param_Bekalici);
		P_NATIVE_END;
	}
	void UBPFL::StaticRegisterNativesUBPFL()
	{
		UClass* Class = UBPFL::StaticClass();
		static const FNameNativePtrPair Funcs[] = {
			{ "PaintRandomSMVertices", &UBPFL::execPaintRandomSMVertices },
			{ "PaintSMVertices", &UBPFL::execPaintSMVertices },
			{ "ReturnFromHex", &UBPFL::execReturnFromHex },
		};
		FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
	}
	struct Z_Construct_UFunction_UBPFL_PaintRandomSMVertices_Statics
	{
		struct BPFL_eventPaintRandomSMVertices_Parms
		{
			UStaticMeshComponent* SMComp;
		};
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_SMComp_MetaData[];
#endif
		static const UECodeGen_Private::FObjectPropertyParams NewProp_SMComp;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_UBPFL_PaintRandomSMVertices_Statics::NewProp_SMComp_MetaData[] = {
		{ "EditInline", "true" },
	};
#endif
	const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_UBPFL_PaintRandomSMVertices_Statics::NewProp_SMComp = { "SMComp", nullptr, (EPropertyFlags)0x0010000000080080, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(BPFL_eventPaintRandomSMVertices_Parms, SMComp), Z_Construct_UClass_UStaticMeshComponent_NoRegister, METADATA_PARAMS(Z_Construct_UFunction_UBPFL_PaintRandomSMVertices_Statics::NewProp_SMComp_MetaData, UE_ARRAY_COUNT(Z_Construct_UFunction_UBPFL_PaintRandomSMVertices_Statics::NewProp_SMComp_MetaData)) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UBPFL_PaintRandomSMVertices_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UBPFL_PaintRandomSMVertices_Statics::NewProp_SMComp,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_UBPFL_PaintRandomSMVertices_Statics::Function_MetaDataParams[] = {
		{ "Category", "VertexPainting" },
		{ "ModuleRelativePath", "Public/BPFL.h" },
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UBPFL_PaintRandomSMVertices_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UBPFL, nullptr, "PaintRandomSMVertices", nullptr, nullptr, sizeof(Z_Construct_UFunction_UBPFL_PaintRandomSMVertices_Statics::BPFL_eventPaintRandomSMVertices_Parms), Z_Construct_UFunction_UBPFL_PaintRandomSMVertices_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UBPFL_PaintRandomSMVertices_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_UBPFL_PaintRandomSMVertices_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_UBPFL_PaintRandomSMVertices_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_UBPFL_PaintRandomSMVertices()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UBPFL_PaintRandomSMVertices_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics
	{
		struct BPFL_eventPaintSMVertices_Parms
		{
			UStaticMeshComponent* SMComp;
			TArray<FColor> Bekalici;
		};
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_SMComp_MetaData[];
#endif
		static const UECodeGen_Private::FObjectPropertyParams NewProp_SMComp;
		static const UECodeGen_Private::FStructPropertyParams NewProp_Bekalici_Inner;
		static const UECodeGen_Private::FArrayPropertyParams NewProp_Bekalici;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics::NewProp_SMComp_MetaData[] = {
		{ "EditInline", "true" },
	};
#endif
	const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics::NewProp_SMComp = { "SMComp", nullptr, (EPropertyFlags)0x0010000000080080, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(BPFL_eventPaintSMVertices_Parms, SMComp), Z_Construct_UClass_UStaticMeshComponent_NoRegister, METADATA_PARAMS(Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics::NewProp_SMComp_MetaData, UE_ARRAY_COUNT(Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics::NewProp_SMComp_MetaData)) };
	const UECodeGen_Private::FStructPropertyParams Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics::NewProp_Bekalici_Inner = { "Bekalici", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, Z_Construct_UScriptStruct_FColor, METADATA_PARAMS(nullptr, 0) };
	const UECodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics::NewProp_Bekalici = { "Bekalici", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(BPFL_eventPaintSMVertices_Parms, Bekalici), EArrayPropertyFlags::None, METADATA_PARAMS(nullptr, 0) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics::NewProp_SMComp,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics::NewProp_Bekalici_Inner,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics::NewProp_Bekalici,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics::Function_MetaDataParams[] = {
		{ "Category", "VertexPainting" },
		{ "ModuleRelativePath", "Public/BPFL.h" },
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UBPFL, nullptr, "PaintSMVertices", nullptr, nullptr, sizeof(Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics::BPFL_eventPaintSMVertices_Parms), Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_UBPFL_PaintSMVertices()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UBPFL_PaintSMVertices_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_UBPFL_ReturnFromHex_Statics
	{
		struct BPFL_eventReturnFromHex_Parms
		{
			FString Beka;
			FColor ReturnValue;
		};
		static const UECodeGen_Private::FStrPropertyParams NewProp_Beka;
		static const UECodeGen_Private::FStructPropertyParams NewProp_ReturnValue;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
	const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UBPFL_ReturnFromHex_Statics::NewProp_Beka = { "Beka", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(BPFL_eventReturnFromHex_Parms, Beka), METADATA_PARAMS(nullptr, 0) };
	const UECodeGen_Private::FStructPropertyParams Z_Construct_UFunction_UBPFL_ReturnFromHex_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(BPFL_eventReturnFromHex_Parms, ReturnValue), Z_Construct_UScriptStruct_FColor, METADATA_PARAMS(nullptr, 0) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UBPFL_ReturnFromHex_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UBPFL_ReturnFromHex_Statics::NewProp_Beka,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UBPFL_ReturnFromHex_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_UBPFL_ReturnFromHex_Statics::Function_MetaDataParams[] = {
		{ "Category", "VertexPainting" },
		{ "ModuleRelativePath", "Public/BPFL.h" },
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UBPFL_ReturnFromHex_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UBPFL, nullptr, "ReturnFromHex", nullptr, nullptr, sizeof(Z_Construct_UFunction_UBPFL_ReturnFromHex_Statics::BPFL_eventReturnFromHex_Parms), Z_Construct_UFunction_UBPFL_ReturnFromHex_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UBPFL_ReturnFromHex_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04822401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_UBPFL_ReturnFromHex_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_UBPFL_ReturnFromHex_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_UBPFL_ReturnFromHex()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UBPFL_ReturnFromHex_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UBPFL);
	UClass* Z_Construct_UClass_UBPFL_NoRegister()
	{
		return UBPFL::StaticClass();
	}
	struct Z_Construct_UClass_UBPFL_Statics
	{
		static UObject* (*const DependentSingletons[])();
		static const FClassFunctionLinkInfo FuncInfo[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UECodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UBPFL_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UBlueprintFunctionLibrary,
		(UObject* (*)())Z_Construct_UPackage__Script_Uiana,
	};
	const FClassFunctionLinkInfo Z_Construct_UClass_UBPFL_Statics::FuncInfo[] = {
		{ &Z_Construct_UFunction_UBPFL_PaintRandomSMVertices, "PaintRandomSMVertices" }, // 1378245115
		{ &Z_Construct_UFunction_UBPFL_PaintSMVertices, "PaintSMVertices" }, // 2888499001
		{ &Z_Construct_UFunction_UBPFL_ReturnFromHex, "ReturnFromHex" }, // 4252485832
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UBPFL_Statics::Class_MetaDataParams[] = {
		{ "Comment", "/**\n * \n */" },
		{ "IncludePath", "BPFL.h" },
		{ "ModuleRelativePath", "Public/BPFL.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_UBPFL_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UBPFL>::IsAbstract,
	};
	const UECodeGen_Private::FClassParams Z_Construct_UClass_UBPFL_Statics::ClassParams = {
		&UBPFL::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		FuncInfo,
		nullptr,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		UE_ARRAY_COUNT(FuncInfo),
		0,
		0,
		0x001000A0u,
		METADATA_PARAMS(Z_Construct_UClass_UBPFL_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_UBPFL_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UBPFL()
	{
		if (!Z_Registration_Info_UClass_UBPFL.OuterSingleton)
		{
			UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UBPFL.OuterSingleton, Z_Construct_UClass_UBPFL_Statics::ClassParams);
		}
		return Z_Registration_Info_UClass_UBPFL.OuterSingleton;
	}
	template<> UIANA_API UClass* StaticClass<UBPFL>()
	{
		return UBPFL::StaticClass();
	}
	DEFINE_VTABLE_PTR_HELPER_CTOR(UBPFL);
	struct Z_CompiledInDeferFile_FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_Statics
	{
		static const FClassRegisterCompiledInInfo ClassInfo[];
	};
	const FClassRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_Statics::ClassInfo[] = {
		{ Z_Construct_UClass_UBPFL, UBPFL::StaticClass, TEXT("UBPFL"), &Z_Registration_Info_UClass_UBPFL, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UBPFL), 1585523741U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_2585995952(TEXT("/Script/Uiana"),
		Z_CompiledInDeferFile_FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_BPFL_h_Statics::ClassInfo),
		nullptr, 0,
		nullptr, 0);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
