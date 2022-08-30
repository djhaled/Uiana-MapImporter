// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PSKXFactory.generated.h"

UCLASS()
class UNREALPSKPSA_API UPSKXFactory : public UFactory
{
	GENERATED_BODY()
	
public:
	UPSKXFactory()
	{
		bEditorImport = true;
		bText = false;

		Formats.Add(FactoryExtension + ";" + FactoryDescription);

		SupportedClass = FactoryClass;
	}
	
	UObject* Import(const FString Filename, UObject* Parent, const FName Name, const EObjectFlags Flags) const;
	
	UClass* FactoryClass = UStaticMesh::StaticClass();
	FString FactoryExtension = "pskx";
	FString FactoryDescription = "ActorX Static Mesh";

	virtual bool FactoryCanImport(const FString& Filename) override
	{
		const auto Extension = FPaths::GetExtension(Filename);
		return Extension.Equals(FactoryExtension);
	}
	
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Params, FFeedbackContext* Warn, bool& bOutOperationCanceled) override
	{
		return Import(Filename, InParent, FName(*InName.ToString().Replace(TEXT("_LOD0"), TEXT(""))), Flags);
	}

};
