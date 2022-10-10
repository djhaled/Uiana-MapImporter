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
	RootComponent = nullptr;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
}

// Called when the game starts or when spawned
void AValActor::BeginPlay()
{
	Super::BeginPlay();
	
}




void AValActor::CreateInstanceComponent(UHierarchicalInstancedStaticMeshComponent*& NewComp,  UStaticMesh* MeshToUSE, FTransform TForm)
{
	UHierarchicalInstancedStaticMeshComponent* HiStaticMesh;
	NewComp = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, HiStaticMesh->StaticClass());
	NewComp->SetStaticMesh(MeshToUSE);
	NewComp->SetupAttachment(RootComponent);
	NewComp->RegisterComponent();
	AddInstanceComponent(NewComp);
	NewComp->SetFlags(RF_Transactional);
	NewComp->SetMobility(EComponentMobility::Static);
	NewComp->SetWorldTransform(TForm);
	RootComponent = NewComp;
}

void AValActor::CreateStaticComponent(UStaticMeshComponent*& NewComp,UStaticMesh* MeshToUSE,FTransform TForm)
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
	NewComp->SetWorldTransform(TForm);
	RootComponent = NewComp;
}








