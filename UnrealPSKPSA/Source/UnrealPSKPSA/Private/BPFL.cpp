// Fill out your copyright notice in the Description page of Project Settings.


#include "BPFL.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
//#include "VectorTypes.h"
#include "Engine/StaticMesh.h"
#include "KismetProceduralMeshLibrary.h"
#include "Engine/World.h"
#include "StaticMeshDescription.h"
#include "Materials/MaterialInstance.h"
#include "PSKReader.h"
#include "Engine/RendererSettings.h"
void UBPFL::PaintSMVertices(UStaticMeshComponent* SMComp, TArray<FColor> VtxColorsArray, FString FileName)
{
	TArray<FColor> FinalColors;
	UStaticMesh* SM = SMComp->GetStaticMesh();
	/// here is old script everythign works down here so dont bother
	//Get the static mesh that we're going to paint
	if (SM)
	{
		//Get the vertex buffer from the 1st lod
		//FPositionVertexBuffer* PositionVertexBuffer = &SM->RenderData->LODResources[0].VertexBuffers.PositionVertexBuffer;

		//Make sure that we have at least 1 LOD
		SMComp->SetLODDataCount(1, SMComp->LODData.Num());
		FStaticMeshComponentLODInfo* LODInfo = &SMComp->LODData[0]; //We're going to modify the 1st LOD only
		FStaticMeshLODResources& LodResources = SM->GetRenderData()->LODResources[0];
		auto numverts = LodResources.GetNumVertices();
		//Empty the painted vertices and assign a new color vertex buffer which will contain the new colors for each vertex
		LODInfo->PaintedVertices.Empty();
		LODInfo->OverrideVertexColors = new FColorVertexBuffer();

		// get reader positions /////
		const auto Reader = new PSKReader(FileName);
		Reader->Read();
		TArray<FVector> CurrentVerts = ReturnCurrentVerts(SM);
		if (VtxColorsArray.Num() != CurrentVerts.Num())
		{
			FinalColors = FixBrokenMesh(SM,FileName,VtxColorsArray, Reader->Vertices);

		}
		else
		{
			auto Hashmap = MakeHashmap(Reader->Vertices, VtxColorsArray);
			// Shell_3_AtkCourtyardGroundA
			// Shell_3_AtkPathB
			for (auto vt : CurrentVerts)
			{
				vt.Y = -vt.Y;
				auto idaa = FVector(vt);
				auto finder = Hashmap.Find(idaa);
				if (finder)
				{
					FinalColors.Add(*finder);
				}
			}
		}
		//Since we know beforehand the number of elements we might as well reserve the memory now
		//Initialize the new vertex colros with the array we created above
		if (FinalColors.Num() != CurrentVerts.Num())
		{
			UE_LOG(LogTemp, Warning, TEXT("This one has wrong FinalColors %s"), *SM->GetName());
		}
		if (FinalColors.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("This one has no  FinalColors %s"), *SM->GetName());
			return;
		}
		LODInfo->OverrideVertexColors->InitFromColorArray(FinalColors);

		//Initialize resource and mark render state of object as dirty in order for the engine to re-render it
		BeginInitResource(LODInfo->OverrideVertexColors);
		SMComp->MarkRenderStateDirty();
	}
}

FColor UBPFL::ReturnFromHex(FString Beka)
{
	return FColor::FromHex(Beka);
}

// Copied from https://qiita.com/EGJ-Kaz_Okada/items/4fd6db895b398893cbbb
void UBPFL::SetStaticSwitchParameterValue(UMaterialInstance* Instance, FName ParameterName, bool Value)
{
	FStaticParameterSet StaticParameters = Instance->GetStaticParameters();
	for (auto& SwitchParameter : StaticParameters.StaticSwitchParameters)
	{
		if (SwitchParameter.ParameterInfo.Name == ParameterName)
		{
			SwitchParameter.Value = Value;
			break;
		}
	}
	Instance->UpdateStaticPermutation(StaticParameters);
}

TMap<FVector, FColor> UBPFL::MakeHashmap(TArray<FVector> arr1, TArray<FColor> TestVtx)
{
	//arr1 should be correct reader
	TMap<FVector, FColor> FruitMap;
	FruitMap.Empty();
	int idx = 0;
	for (auto j : arr1)
	{
		FruitMap.Add(j, TestVtx[idx]);
		idx++;
	}
	return FruitMap;
}


TArray<FColor> UBPFL::FixBrokenMesh(UStaticMesh* SMesh, FString ReaderFile, TArray<FColor> BrokenVtxColorArray,TArray<FVector> ReaderVerts)
{
	TArray<FColor> LocalVtxColors;
	// Get Actual Verts
	auto CurrentVerticesPosition = ReturnCurrentVerts(SMesh);
	int32  ida = 0;
	for (auto vtx : ReaderVerts)
	{
		ReaderVerts[ida].Y = -vtx.Y;
		ida++;
	}
	TArray<FColor> VtxOrderedColors;
	int index = -1;
	// from the new array without the missing ones if  equals the vtxpos and hasnt been changed yet add to the vtx colors at the index "correct
	auto Hasher = MakeHashmap(ReaderVerts, BrokenVtxColorArray);
	for (auto vt : CurrentVerticesPosition)
	{
		auto finder = Hasher.Find(FVector(vt));
		LocalVtxColors.Add(*finder);

	}
	return LocalVtxColors;
}

TArray<FVector> UBPFL::ReturnCurrentVerts(UStaticMesh* Mesh)
{
	TArray<FVector> ReturnArray;
	if (Mesh->GetRenderData()->LODResources.Num() > 0)
	{
		FPositionVertexBuffer* VertexBuffer = &Mesh->GetRenderData()->LODResources[0].VertexBuffers.PositionVertexBuffer;
		if (VertexBuffer)
		{
			const int32 VertexCount = VertexBuffer->GetNumVertices();
			for (int32 Index = 0; Index < VertexCount; Index++)
			{
				//This is in the Static Mesh Actor Class, so it is location and tranform of the SMActor
				const FVector Vertex = VertexBuffer->VertexPosition(Index);
				//add to output FVector array
				ReturnArray.Add(Vertex);
			}
		}
	}
	return ReturnArray;
}
void UBPFL::ChangeProjectSettings()
{
	URendererSettings* Settings = GetMutableDefault<URendererSettings>();
	Settings->DefaultLightUnits = ELightUnits::Unitless;
	// Settings->DynamicGlobalIllumination = EDynamicGlobalIlluminationMethod::None;
	// Settings->Reflections == EReflectionMethod::None;
	Settings->SaveConfig();
}