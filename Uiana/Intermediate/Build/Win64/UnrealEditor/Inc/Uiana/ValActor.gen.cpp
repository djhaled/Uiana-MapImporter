// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Uiana/Public/ValActor.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeValActor() {}
// Cross Module References
	UIANA_API UClass* Z_Construct_UClass_AValActor_NoRegister();
	UIANA_API UClass* Z_Construct_UClass_AValActor();
	ENGINE_API UClass* Z_Construct_UClass_AActor();
	UPackage* Z_Construct_UPackage__Script_Uiana();
	ENGINE_API UClass* Z_Construct_UClass_UBillboardComponent_NoRegister();
	ENGINE_API UClass* Z_Construct_UClass_UStaticMeshComponent_NoRegister();
	ENGINE_API UClass* Z_Construct_UClass_UBoxComponent_NoRegister();
	ENGINE_API UClass* Z_Construct_UClass_UCapsuleComponent_NoRegister();
	ENGINE_API UClass* Z_Construct_UClass_UHierarchicalInstancedStaticMeshComponent_NoRegister();
	COREUOBJECT_API UScriptStruct* Z_Construct_UScriptStruct_FColor();
	ENGINE_API UClass* Z_Construct_UClass_UStaticMesh_NoRegister();
	ENGINE_API UClass* Z_Construct_UClass_UMaterialInstanceConstant_NoRegister();
	ENGINE_API UClass* Z_Construct_UClass_USceneComponent_NoRegister();
// End Cross Module References
	DEFINE_FUNCTION(AValActor::execCreateBlockingVolumeComponent)
	{
		P_GET_OBJECT_REF(UStaticMeshComponent,Z_Param_Out_NewComp);
		P_FINISH;
		P_NATIVE_BEGIN;
		P_THIS->CreateBlockingVolumeComponent(Z_Param_Out_NewComp);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(AValActor::execCreateBillboardComponent)
	{
		P_GET_OBJECT_REF(UBillboardComponent,Z_Param_Out_NewComp);
		P_FINISH;
		P_NATIVE_BEGIN;
		P_THIS->CreateBillboardComponent(Z_Param_Out_NewComp);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(AValActor::execCreateBoxComponent)
	{
		P_GET_OBJECT_REF(UBoxComponent,Z_Param_Out_NewComp);
		P_FINISH;
		P_NATIVE_BEGIN;
		P_THIS->CreateBoxComponent(Z_Param_Out_NewComp);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(AValActor::execCreateStaticComponent)
	{
		P_GET_OBJECT_REF(UStaticMeshComponent,Z_Param_Out_NewComp);
		P_GET_TARRAY(FColor,Z_Param_OvrVertex);
		P_GET_OBJECT(UStaticMesh,Z_Param_MeshToUSE);
		P_FINISH;
		P_NATIVE_BEGIN;
		P_THIS->CreateStaticComponent(Z_Param_Out_NewComp,Z_Param_OvrVertex,Z_Param_MeshToUSE);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(AValActor::execCreateInstanceComponent)
	{
		P_GET_OBJECT_REF(UHierarchicalInstancedStaticMeshComponent,Z_Param_Out_NewComp);
		P_GET_TARRAY(FColor,Z_Param_OvrVertex);
		P_GET_OBJECT(UStaticMesh,Z_Param_MeshToUSE);
		P_FINISH;
		P_NATIVE_BEGIN;
		P_THIS->CreateInstanceComponent(Z_Param_Out_NewComp,Z_Param_OvrVertex,Z_Param_MeshToUSE);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(AValActor::execCreateCapsuleComponent)
	{
		P_GET_OBJECT_REF(UCapsuleComponent,Z_Param_Out_NewComp);
		P_FINISH;
		P_NATIVE_BEGIN;
		P_THIS->CreateCapsuleComponent(Z_Param_Out_NewComp);
		P_NATIVE_END;
	}
	void AValActor::StaticRegisterNativesAValActor()
	{
		UClass* Class = AValActor::StaticClass();
		static const FNameNativePtrPair Funcs[] = {
			{ "CreateBillboardComponent", &AValActor::execCreateBillboardComponent },
			{ "CreateBlockingVolumeComponent", &AValActor::execCreateBlockingVolumeComponent },
			{ "CreateBoxComponent", &AValActor::execCreateBoxComponent },
			{ "CreateCapsuleComponent", &AValActor::execCreateCapsuleComponent },
			{ "CreateInstanceComponent", &AValActor::execCreateInstanceComponent },
			{ "CreateStaticComponent", &AValActor::execCreateStaticComponent },
		};
		FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
	}
	struct Z_Construct_UFunction_AValActor_CreateBillboardComponent_Statics
	{
		struct ValActor_eventCreateBillboardComponent_Parms
		{
			UBillboardComponent* NewComp;
		};
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_NewComp_MetaData[];
#endif
		static const UECodeGen_Private::FObjectPropertyParams NewProp_NewComp;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AValActor_CreateBillboardComponent_Statics::NewProp_NewComp_MetaData[] = {
		{ "EditInline", "true" },
	};
#endif
	const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_AValActor_CreateBillboardComponent_Statics::NewProp_NewComp = { "NewComp", nullptr, (EPropertyFlags)0x0010000000080180, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ValActor_eventCreateBillboardComponent_Parms, NewComp), Z_Construct_UClass_UBillboardComponent_NoRegister, METADATA_PARAMS(Z_Construct_UFunction_AValActor_CreateBillboardComponent_Statics::NewProp_NewComp_MetaData, UE_ARRAY_COUNT(Z_Construct_UFunction_AValActor_CreateBillboardComponent_Statics::NewProp_NewComp_MetaData)) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_AValActor_CreateBillboardComponent_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AValActor_CreateBillboardComponent_Statics::NewProp_NewComp,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AValActor_CreateBillboardComponent_Statics::Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/ValActor.h" },
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AValActor_CreateBillboardComponent_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_AValActor, nullptr, "CreateBillboardComponent", nullptr, nullptr, sizeof(Z_Construct_UFunction_AValActor_CreateBillboardComponent_Statics::ValActor_eventCreateBillboardComponent_Parms), Z_Construct_UFunction_AValActor_CreateBillboardComponent_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_AValActor_CreateBillboardComponent_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04420401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_AValActor_CreateBillboardComponent_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_AValActor_CreateBillboardComponent_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_AValActor_CreateBillboardComponent()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AValActor_CreateBillboardComponent_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_AValActor_CreateBlockingVolumeComponent_Statics
	{
		struct ValActor_eventCreateBlockingVolumeComponent_Parms
		{
			UStaticMeshComponent* NewComp;
		};
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_NewComp_MetaData[];
#endif
		static const UECodeGen_Private::FObjectPropertyParams NewProp_NewComp;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AValActor_CreateBlockingVolumeComponent_Statics::NewProp_NewComp_MetaData[] = {
		{ "EditInline", "true" },
	};
#endif
	const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_AValActor_CreateBlockingVolumeComponent_Statics::NewProp_NewComp = { "NewComp", nullptr, (EPropertyFlags)0x0010000000080180, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ValActor_eventCreateBlockingVolumeComponent_Parms, NewComp), Z_Construct_UClass_UStaticMeshComponent_NoRegister, METADATA_PARAMS(Z_Construct_UFunction_AValActor_CreateBlockingVolumeComponent_Statics::NewProp_NewComp_MetaData, UE_ARRAY_COUNT(Z_Construct_UFunction_AValActor_CreateBlockingVolumeComponent_Statics::NewProp_NewComp_MetaData)) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_AValActor_CreateBlockingVolumeComponent_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AValActor_CreateBlockingVolumeComponent_Statics::NewProp_NewComp,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AValActor_CreateBlockingVolumeComponent_Statics::Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/ValActor.h" },
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AValActor_CreateBlockingVolumeComponent_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_AValActor, nullptr, "CreateBlockingVolumeComponent", nullptr, nullptr, sizeof(Z_Construct_UFunction_AValActor_CreateBlockingVolumeComponent_Statics::ValActor_eventCreateBlockingVolumeComponent_Parms), Z_Construct_UFunction_AValActor_CreateBlockingVolumeComponent_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_AValActor_CreateBlockingVolumeComponent_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04420401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_AValActor_CreateBlockingVolumeComponent_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_AValActor_CreateBlockingVolumeComponent_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_AValActor_CreateBlockingVolumeComponent()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AValActor_CreateBlockingVolumeComponent_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_AValActor_CreateBoxComponent_Statics
	{
		struct ValActor_eventCreateBoxComponent_Parms
		{
			UBoxComponent* NewComp;
		};
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_NewComp_MetaData[];
#endif
		static const UECodeGen_Private::FObjectPropertyParams NewProp_NewComp;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AValActor_CreateBoxComponent_Statics::NewProp_NewComp_MetaData[] = {
		{ "EditInline", "true" },
	};
#endif
	const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_AValActor_CreateBoxComponent_Statics::NewProp_NewComp = { "NewComp", nullptr, (EPropertyFlags)0x0010000000080180, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ValActor_eventCreateBoxComponent_Parms, NewComp), Z_Construct_UClass_UBoxComponent_NoRegister, METADATA_PARAMS(Z_Construct_UFunction_AValActor_CreateBoxComponent_Statics::NewProp_NewComp_MetaData, UE_ARRAY_COUNT(Z_Construct_UFunction_AValActor_CreateBoxComponent_Statics::NewProp_NewComp_MetaData)) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_AValActor_CreateBoxComponent_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AValActor_CreateBoxComponent_Statics::NewProp_NewComp,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AValActor_CreateBoxComponent_Statics::Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/ValActor.h" },
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AValActor_CreateBoxComponent_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_AValActor, nullptr, "CreateBoxComponent", nullptr, nullptr, sizeof(Z_Construct_UFunction_AValActor_CreateBoxComponent_Statics::ValActor_eventCreateBoxComponent_Parms), Z_Construct_UFunction_AValActor_CreateBoxComponent_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_AValActor_CreateBoxComponent_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04420401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_AValActor_CreateBoxComponent_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_AValActor_CreateBoxComponent_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_AValActor_CreateBoxComponent()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AValActor_CreateBoxComponent_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_AValActor_CreateCapsuleComponent_Statics
	{
		struct ValActor_eventCreateCapsuleComponent_Parms
		{
			UCapsuleComponent* NewComp;
		};
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_NewComp_MetaData[];
#endif
		static const UECodeGen_Private::FObjectPropertyParams NewProp_NewComp;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AValActor_CreateCapsuleComponent_Statics::NewProp_NewComp_MetaData[] = {
		{ "EditInline", "true" },
	};
#endif
	const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_AValActor_CreateCapsuleComponent_Statics::NewProp_NewComp = { "NewComp", nullptr, (EPropertyFlags)0x0010000000080180, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ValActor_eventCreateCapsuleComponent_Parms, NewComp), Z_Construct_UClass_UCapsuleComponent_NoRegister, METADATA_PARAMS(Z_Construct_UFunction_AValActor_CreateCapsuleComponent_Statics::NewProp_NewComp_MetaData, UE_ARRAY_COUNT(Z_Construct_UFunction_AValActor_CreateCapsuleComponent_Statics::NewProp_NewComp_MetaData)) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_AValActor_CreateCapsuleComponent_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AValActor_CreateCapsuleComponent_Statics::NewProp_NewComp,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AValActor_CreateCapsuleComponent_Statics::Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/ValActor.h" },
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AValActor_CreateCapsuleComponent_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_AValActor, nullptr, "CreateCapsuleComponent", nullptr, nullptr, sizeof(Z_Construct_UFunction_AValActor_CreateCapsuleComponent_Statics::ValActor_eventCreateCapsuleComponent_Parms), Z_Construct_UFunction_AValActor_CreateCapsuleComponent_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_AValActor_CreateCapsuleComponent_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04420401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_AValActor_CreateCapsuleComponent_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_AValActor_CreateCapsuleComponent_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_AValActor_CreateCapsuleComponent()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AValActor_CreateCapsuleComponent_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics
	{
		struct ValActor_eventCreateInstanceComponent_Parms
		{
			UHierarchicalInstancedStaticMeshComponent* NewComp;
			TArray<FColor> OvrVertex;
			UStaticMesh* MeshToUSE;
		};
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_NewComp_MetaData[];
#endif
		static const UECodeGen_Private::FObjectPropertyParams NewProp_NewComp;
		static const UECodeGen_Private::FStructPropertyParams NewProp_OvrVertex_Inner;
		static const UECodeGen_Private::FArrayPropertyParams NewProp_OvrVertex;
		static const UECodeGen_Private::FObjectPropertyParams NewProp_MeshToUSE;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::NewProp_NewComp_MetaData[] = {
		{ "EditInline", "true" },
	};
#endif
	const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::NewProp_NewComp = { "NewComp", nullptr, (EPropertyFlags)0x0010000000080180, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ValActor_eventCreateInstanceComponent_Parms, NewComp), Z_Construct_UClass_UHierarchicalInstancedStaticMeshComponent_NoRegister, METADATA_PARAMS(Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::NewProp_NewComp_MetaData, UE_ARRAY_COUNT(Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::NewProp_NewComp_MetaData)) };
	const UECodeGen_Private::FStructPropertyParams Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::NewProp_OvrVertex_Inner = { "OvrVertex", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, Z_Construct_UScriptStruct_FColor, METADATA_PARAMS(nullptr, 0) };
	const UECodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::NewProp_OvrVertex = { "OvrVertex", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ValActor_eventCreateInstanceComponent_Parms, OvrVertex), EArrayPropertyFlags::None, METADATA_PARAMS(nullptr, 0) };
	const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::NewProp_MeshToUSE = { "MeshToUSE", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ValActor_eventCreateInstanceComponent_Parms, MeshToUSE), Z_Construct_UClass_UStaticMesh_NoRegister, METADATA_PARAMS(nullptr, 0) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::NewProp_NewComp,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::NewProp_OvrVertex_Inner,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::NewProp_OvrVertex,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::NewProp_MeshToUSE,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/ValActor.h" },
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_AValActor, nullptr, "CreateInstanceComponent", nullptr, nullptr, sizeof(Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::ValActor_eventCreateInstanceComponent_Parms), Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04420401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_AValActor_CreateInstanceComponent()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AValActor_CreateInstanceComponent_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics
	{
		struct ValActor_eventCreateStaticComponent_Parms
		{
			UStaticMeshComponent* NewComp;
			TArray<FColor> OvrVertex;
			UStaticMesh* MeshToUSE;
		};
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_NewComp_MetaData[];
#endif
		static const UECodeGen_Private::FObjectPropertyParams NewProp_NewComp;
		static const UECodeGen_Private::FStructPropertyParams NewProp_OvrVertex_Inner;
		static const UECodeGen_Private::FArrayPropertyParams NewProp_OvrVertex;
		static const UECodeGen_Private::FObjectPropertyParams NewProp_MeshToUSE;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::NewProp_NewComp_MetaData[] = {
		{ "EditInline", "true" },
	};
#endif
	const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::NewProp_NewComp = { "NewComp", nullptr, (EPropertyFlags)0x0010000000080180, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ValActor_eventCreateStaticComponent_Parms, NewComp), Z_Construct_UClass_UStaticMeshComponent_NoRegister, METADATA_PARAMS(Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::NewProp_NewComp_MetaData, UE_ARRAY_COUNT(Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::NewProp_NewComp_MetaData)) };
	const UECodeGen_Private::FStructPropertyParams Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::NewProp_OvrVertex_Inner = { "OvrVertex", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, Z_Construct_UScriptStruct_FColor, METADATA_PARAMS(nullptr, 0) };
	const UECodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::NewProp_OvrVertex = { "OvrVertex", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ValActor_eventCreateStaticComponent_Parms, OvrVertex), EArrayPropertyFlags::None, METADATA_PARAMS(nullptr, 0) };
	const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::NewProp_MeshToUSE = { "MeshToUSE", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ValActor_eventCreateStaticComponent_Parms, MeshToUSE), Z_Construct_UClass_UStaticMesh_NoRegister, METADATA_PARAMS(nullptr, 0) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::NewProp_NewComp,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::NewProp_OvrVertex_Inner,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::NewProp_OvrVertex,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::NewProp_MeshToUSE,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/ValActor.h" },
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_AValActor, nullptr, "CreateStaticComponent", nullptr, nullptr, sizeof(Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::ValActor_eventCreateStaticComponent_Parms), Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04420401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_AValActor_CreateStaticComponent()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AValActor_CreateStaticComponent_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(AValActor);
	UClass* Z_Construct_UClass_AValActor_NoRegister()
	{
		return AValActor::StaticClass();
	}
	struct Z_Construct_UClass_AValActor_Statics
	{
		static UObject* (*const DependentSingletons[])();
		static const FClassFunctionLinkInfo FuncInfo[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const UECodeGen_Private::FObjectPropertyParams NewProp_FruitMap_ValueProp;
		static const UECodeGen_Private::FNamePropertyParams NewProp_FruitMap_Key_KeyProp;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_FruitMap_MetaData[];
#endif
		static const UECodeGen_Private::FMapPropertyParams NewProp_FruitMap;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_SceneComp_MetaData[];
#endif
		static const UECodeGen_Private::FObjectPropertyParams NewProp_SceneComp;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UECodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_AValActor_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_AActor,
		(UObject* (*)())Z_Construct_UPackage__Script_Uiana,
	};
	const FClassFunctionLinkInfo Z_Construct_UClass_AValActor_Statics::FuncInfo[] = {
		{ &Z_Construct_UFunction_AValActor_CreateBillboardComponent, "CreateBillboardComponent" }, // 767021628
		{ &Z_Construct_UFunction_AValActor_CreateBlockingVolumeComponent, "CreateBlockingVolumeComponent" }, // 644443586
		{ &Z_Construct_UFunction_AValActor_CreateBoxComponent, "CreateBoxComponent" }, // 2887696274
		{ &Z_Construct_UFunction_AValActor_CreateCapsuleComponent, "CreateCapsuleComponent" }, // 1843307221
		{ &Z_Construct_UFunction_AValActor_CreateInstanceComponent, "CreateInstanceComponent" }, // 3093376394
		{ &Z_Construct_UFunction_AValActor_CreateStaticComponent, "CreateStaticComponent" }, // 439996065
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_AValActor_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "ValActor.h" },
		{ "ModuleRelativePath", "Public/ValActor.h" },
	};
#endif
	const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AValActor_Statics::NewProp_FruitMap_ValueProp = { "FruitMap", nullptr, (EPropertyFlags)0x0000000000000001, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, 1, Z_Construct_UClass_UMaterialInstanceConstant_NoRegister, METADATA_PARAMS(nullptr, 0) };
	const UECodeGen_Private::FNamePropertyParams Z_Construct_UClass_AValActor_Statics::NewProp_FruitMap_Key_KeyProp = { "FruitMap_Key", nullptr, (EPropertyFlags)0x0000000000000001, UECodeGen_Private::EPropertyGenFlags::Name, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_AValActor_Statics::NewProp_FruitMap_MetaData[] = {
		{ "Category", "MapsAndSets" },
		{ "ModuleRelativePath", "Public/ValActor.h" },
	};
#endif
	const UECodeGen_Private::FMapPropertyParams Z_Construct_UClass_AValActor_Statics::NewProp_FruitMap = { "FruitMap", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Map, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(AValActor, FruitMap), EMapPropertyFlags::None, METADATA_PARAMS(Z_Construct_UClass_AValActor_Statics::NewProp_FruitMap_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_AValActor_Statics::NewProp_FruitMap_MetaData)) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_AValActor_Statics::NewProp_SceneComp_MetaData[] = {
		{ "Category", "Scene" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/ValActor.h" },
	};
#endif
	const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AValActor_Statics::NewProp_SceneComp = { "SceneComp", nullptr, (EPropertyFlags)0x00100000000a001d, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(AValActor, SceneComp), Z_Construct_UClass_USceneComponent_NoRegister, METADATA_PARAMS(Z_Construct_UClass_AValActor_Statics::NewProp_SceneComp_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_AValActor_Statics::NewProp_SceneComp_MetaData)) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_AValActor_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AValActor_Statics::NewProp_FruitMap_ValueProp,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AValActor_Statics::NewProp_FruitMap_Key_KeyProp,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AValActor_Statics::NewProp_FruitMap,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AValActor_Statics::NewProp_SceneComp,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_AValActor_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AValActor>::IsAbstract,
	};
	const UECodeGen_Private::FClassParams Z_Construct_UClass_AValActor_Statics::ClassParams = {
		&AValActor::StaticClass,
		"Engine",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		FuncInfo,
		Z_Construct_UClass_AValActor_Statics::PropPointers,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		UE_ARRAY_COUNT(FuncInfo),
		UE_ARRAY_COUNT(Z_Construct_UClass_AValActor_Statics::PropPointers),
		0,
		0x009000A4u,
		METADATA_PARAMS(Z_Construct_UClass_AValActor_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_AValActor_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_AValActor()
	{
		if (!Z_Registration_Info_UClass_AValActor.OuterSingleton)
		{
			UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AValActor.OuterSingleton, Z_Construct_UClass_AValActor_Statics::ClassParams);
		}
		return Z_Registration_Info_UClass_AValActor.OuterSingleton;
	}
	template<> UIANA_API UClass* StaticClass<AValActor>()
	{
		return AValActor::StaticClass();
	}
	DEFINE_VTABLE_PTR_HELPER_CTOR(AValActor);
	struct Z_CompiledInDeferFile_FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_ValActor_h_Statics
	{
		static const FClassRegisterCompiledInInfo ClassInfo[];
	};
	const FClassRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_ValActor_h_Statics::ClassInfo[] = {
		{ Z_Construct_UClass_AValActor, AValActor::StaticClass, TEXT("AValActor"), &Z_Registration_Info_UClass_AValActor, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AValActor), 3513461290U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_ValActor_h_1535498031(TEXT("/Script/Uiana"),
		Z_CompiledInDeferFile_FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_ValActor_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ProjectBeka_Plugins_Uiana_Source_Uiana_Public_ValActor_h_Statics::ClassInfo),
		nullptr, 0,
		nullptr, 0);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
