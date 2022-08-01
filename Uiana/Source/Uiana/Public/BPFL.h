// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BPFL.generated.h"

/**
 * 
 */
UCLASS()
class UIANA_API UBPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = VertexPainting)
	static void PaintSMVertices(UStaticMeshComponent* SMComp, TArray<FColor> Bekalici);
	UFUNCTION(BlueprintCallable, Category = VertexPainting)
	static FColor ReturnFromHex(FString Beka);
<<<<<<< Updated upstream:Uiana/Source/Uiana/Public/BPFL.h
	UFUNCTION(BlueprintCallable, Category = VertexPainting)
	static void PaintRandomSMVertices(UStaticMeshComponent* SMComp);
=======
	static TMap<FVector3f, FColor> MakeHashmap(TArray<FVector3f> arr1, TArray<FColor> TestVtx);
	static TArray<FColor> FixBrokenMesh(UStaticMesh* SMesh, FString ReaderFile, TArray<FColor> BrokenVtxColorArray, TArray<FVector3f> ReaderVerts);
	static TArray<FVector3f> ReturnCurrentVerts(UStaticMesh* Mesh);
<<<<<<< Updated upstream:Uiana/Source/Uiana/Public/BPFL.h
>>>>>>> Stashed changes:UnrealPSKPSA/Source/UnrealPSKPSA/Public/BPFL.h
=======
>>>>>>> Stashed changes:UnrealPSKPSA/Source/UnrealPSKPSA/Public/BPFL.h
};
