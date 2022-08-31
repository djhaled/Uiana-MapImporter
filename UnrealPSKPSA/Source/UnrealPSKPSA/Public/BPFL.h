// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BPFL.generated.h"

/**
 * 
 */
class AActor;
class UPSKXFactory;

UCLASS()
class UNREALPSKPSA_API UBPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = VertexPainting)
		static UActorComponent* CreateBPComp(UObject* Object, UClass* ClassToUse, FName CompName);
	UFUNCTION(BlueprintCallable, Category = VertexPainting)
	static void PaintSMVertices(UStaticMeshComponent* SMComp, TArray<FColor> VtxColorsArray, FString FileName);
	UFUNCTION(BlueprintCallable, Category = VertexPainting)
	static FColor ReturnFromHex(FString Beka);
	static TMap<FVector3f, FColor> MakeHashmap(TArray<FVector3f> arr1, TArray<FColor> TestVtx);
	static TArray<FColor> FixBrokenMesh(UStaticMesh* SMesh, FString ReaderFile, TArray<FColor> BrokenVtxColorArray, TArray<FVector3f> ReaderVerts);
	static TArray<FVector3f> ReturnCurrentVerts(UStaticMesh* Mesh);
	UFUNCTION(BlueprintCallable, Category = ProjectSettings)
	static void ChangeProjectSettings();
	UFUNCTION(BlueprintCallable, Category = ProjectSettings)
	static void ImportTextures(TArray<FString> AllTexturesPath);
	UFUNCTION(BlueprintCallable, Category = ProjectSettings)
	static void ImportMeshes(TArray<FString> AllMeshesPath, FString ObjectsPath);
	static ECollisionTraceFlag GetTraceFlag(FString tflag);
};
