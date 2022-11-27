// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialInstanceConstant.h"
#include "GameFramework/Actor.h"
#include "HismActorCPP.generated.h"
class HismComponent;
UCLASS()
class UIANACPP_API AHismActorCPP : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHismActorCPP();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY(Category = InstancedStaticMeshActor, VisibleAnywhere, BlueprintReadOnly, meta = (ExposeFunctionCategories = "Mesh,Rendering,Physics,Components|StaticMesh", AllowPrivateAccess = "true"))
	TObjectPtr<class UHierarchicalInstancedStaticMeshComponent> HismComponent;
	class UHierarchicalInstancedStaticMeshComponent* GetStaticMeshComponent() const { return HismComponent; }
};
