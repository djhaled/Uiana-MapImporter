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
class USCS_Node;
class UBrushComponent;

UCLASS()
class UNREALPSKPSA_API UBPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = BPFL)
	static UActorComponent* GetComponentByName(AActor* Actor, FName CompName);
	UFUNCTION(BlueprintCallable, Category = BPFL)
	static void SetOverrideMaterial(AActor* Actor, FName CompName, TArray<UMaterialInterface*> MatOvr);
	UFUNCTION(BlueprintCallable, Category = BPFL)
	static USCS_Node* CreateNode(UObject* Object, UClass* ClassToUse, FName CompName, UActorComponent*& ComponentReturn);
	UFUNCTION(BlueprintCallable, Category = BPFL)
	static UActorComponent* CreateBPComp(UObject* Object, UClass* ClassToUse, FName CompName, TArray<USCS_Node*> AttachNodes);
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
	static UActorComponent* GetComponent(AActor* Actor);
	UFUNCTION(BlueprintCallable, Category = ProjectSettings)
	static void ImportTextures(TArray<FString> AllTexturesPath);
	UFUNCTION(BlueprintCallable, Category = ProjectSettings)
	static void ImportMeshes(TArray<FString> AllMeshesPath, FString ObjectsPath);
	static ECollisionTraceFlag GetTraceFlag(FString tflag);
	UFUNCTION(BlueprintCallable, Category = "Unreal Python")
		static void ExecuteConsoleCommand(FString ConsoleCommand);
	UFUNCTION(BlueprintCallable, Category = ProjectSettings)
		static UObject* SetMeshReference(FString MeshObjectName, FString MeshType);

};
