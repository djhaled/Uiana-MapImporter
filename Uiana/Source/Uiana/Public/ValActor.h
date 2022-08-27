// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialInstanceConstant.h"
#include "GameFramework/Actor.h"
#include "ValActor.generated.h"
class NewComp;
UCLASS()
class UIANA_API AValActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AValActor();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scene")
	USceneComponent* SceneComp;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	void CreateCapsuleComponent(UCapsuleComponent*& NewComp);
	UFUNCTION(BlueprintCallable)
	void CreateInstanceComponent(UHierarchicalInstancedStaticMeshComponent*& NewComp, UStaticMesh* MeshToUSE);
	UFUNCTION(BlueprintCallable)
	void CreateStaticComponent(UStaticMeshComponent*& NewComp, UStaticMesh* MeshToUSE);
	UFUNCTION(BlueprintCallable)
	void CreateBoxComponent(UBoxComponent*& NewComp);
	UFUNCTION(BlueprintCallable)
	void CreateBillboardComponent(UBillboardComponent*& NewComp);
	UFUNCTION(BlueprintCallable)
	void CreateBlockingVolumeComponent(UStaticMeshComponent*& NewComp);
};
