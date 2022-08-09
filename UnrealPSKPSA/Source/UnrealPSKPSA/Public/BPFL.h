// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BPFL.generated.h"

/**
 * 
 */
class AActor;

UCLASS()
class UNREALPSKPSA_API UBPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = VertexPainting)
	static void PaintSMVertices(UStaticMeshComponent* SMComp, TArray<FColor> VtxColorsArray, FString FileName);
	UFUNCTION(BlueprintCallable, Category = VertexPainting)
	static FColor ReturnFromHex(FString Beka);
	static TMap<FVector3f, FColor> MakeHashmap(TArray<FVector3f> arr1, TArray<FColor> TestVtx);
	static TArray<FColor> FixBrokenMesh(UStaticMesh* SMesh, FString ReaderFile, TArray<FColor> BrokenVtxColorArray, TArray<FVector3f> ReaderVerts);
	static TArray<FVector3f> ReturnCurrentVerts(UStaticMesh* Mesh);
};
