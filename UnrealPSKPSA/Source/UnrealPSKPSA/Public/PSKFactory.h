#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "PSKFactory.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPSKPSA, Log, All);

class UEditorAssetLibrary;
UCLASS()
class UNREALPSKPSA_API UPSKFactory : public UFactory
{
	GENERATED_BODY()
	
public:
	UPSKFactory();
	
	virtual bool DoesSupportClass(UClass* InClass) override;
	virtual UClass * ResolveSupportedClass() override;
	virtual bool FactoryCanImport(const FString& InSystemFilePath) override;
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;

	UObject* ImportPSKX(const FString Filename, UObject* Parent, const FName Name, const EObjectFlags Flags);
	
};
