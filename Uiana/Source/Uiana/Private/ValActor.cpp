// Fill out your copyright notice in the Description page of Project Settings.


#include "ValActor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Rendering/ColorVertexBuffer.h"

// Sets default values
AValActor::AValActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SceneComp = CreateDefaultSubobject<USceneComponent>("RootSceneComp");
	RootComponent = SceneComp;
	SceneComp->Mobility = EComponentMobility::Static;
}

// Called when the game starts or when spawned
void AValActor::BeginPlay()
{
	Super::BeginPlay();
}


void AValActor::CreateCapsuleComponent(UCapsuleComponent*& NewComp)
{
	UCapsuleComponent* HiSplineComp;
	NewComp = NewObject<UCapsuleComponent>(this, HiSplineComp->StaticClass());
	NewComp->SetupAttachment(RootComponent);
	NewComp->RegisterComponent();
	AddInstanceComponent(NewComp);
	NewComp->SetFlags(RF_Transactional);
}

void AValActor::CreateInstanceComponent(UHierarchicalInstancedStaticMeshComponent*& NewComp, UStaticMesh* MeshToUSE)
{
	UHierarchicalInstancedStaticMeshComponent* HiStaticMesh;
	NewComp = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, HiStaticMesh->StaticClass());
	NewComp->SetStaticMesh(MeshToUSE);
	NewComp->SetupAttachment(RootComponent);
	NewComp->RegisterComponent();
	AddInstanceComponent(NewComp);
	NewComp->SetFlags(RF_Transactional);
	NewComp->SetMobility(EComponentMobility::Static);
}

void AValActor::CreateStaticComponent(UStaticMeshComponent*& NewComp, UStaticMesh* MeshToUSE)
{
	UStaticMeshComponent* HiStaticMesh;
	NewComp = NewObject<UStaticMeshComponent>(this, HiStaticMesh->StaticClass());
	NewComp->SetStaticMesh(MeshToUSE);
	NewComp->SetupAttachment(RootComponent);
	NewComp->RegisterComponent();
	AddInstanceComponent(NewComp);
	NewComp->SetFlags(RF_Transactional);
	NewComp->SetLODDataCount(1, NewComp->LODData.Num());
	NewComp->SetMobility(EComponentMobility::Static);
}

void AValActor::CreateBoxComponent(UBoxComponent*& NewComp)
{
	UBoxComponent* HiStaticMesh;
	NewComp = NewObject<UBoxComponent>(this, HiStaticMesh->StaticClass());
	NewComp->SetupAttachment(RootComponent);
	NewComp->RegisterComponent();
	AddInstanceComponent(NewComp);
	NewComp->SetFlags(RF_Transactional);
}

void AValActor::CreateBillboardComponent(UBillboardComponent*& NewComp)
{
	UBillboardComponent* HiStaticMesh;
	NewComp = NewObject<UBillboardComponent>(this, HiStaticMesh->StaticClass());
	NewComp->SetupAttachment(RootComponent);
	NewComp->RegisterComponent();
	AddInstanceComponent(NewComp);
	NewComp->SetFlags(RF_Transactional);
}

void AValActor::CreateBlockingVolumeComponent(UStaticMeshComponent*& NewComp)
{
	UStaticMeshComponent* HiStaticMesh;
	NewComp = NewObject<UStaticMeshComponent>(this, HiStaticMesh->StaticClass());
	NewComp->SetupAttachment(RootComponent);
	NewComp->RegisterComponent();
	AddInstanceComponent(NewComp);
	NewComp->SetFlags(RF_Transactional);
	NewComp->bOverrideWireframeColor = true;
	NewComp->WireframeColorOverride = FColor(239, 131, 131, 0);
	NewComp->CastShadow = false;
	NewComp->bHiddenInGame = true;
	NewComp->BodyInstance.MassScale = 177.82793;
	NewComp->SetMobility(EComponentMobility::Static);
}
