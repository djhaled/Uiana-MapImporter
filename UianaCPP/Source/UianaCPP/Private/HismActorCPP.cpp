// Fill out your copyright notice in the Description page of Project Settings.


#include "HismActorCPP.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Rendering/ColorVertexBuffer.h"
#if ENGINE_MAJOR_VERSION == 4
#include "Engine/CollisionProfile.h"
#endif

// Sets default values
AHismActorCPP::AHismActorCPP()
{
	HismComponent = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("InstancedStaticMeshComponent0"));
	HismComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	HismComponent->Mobility = EComponentMobility::Static;
	HismComponent->SetGenerateOverlapEvents(false);
	HismComponent->bUseDefaultCollision = true;
}

// Called when the game starts or when spawned
void AHismActorCPP::BeginPlay()
{
	Super::BeginPlay();
}
