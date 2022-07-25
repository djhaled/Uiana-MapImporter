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
	UFUNCTION(BlueprintCallable, Category = VertexPainting)
	static void PaintRandomSMVertices(UStaticMeshComponent* SMComp);
};
