// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Modules/ModuleManager.h"
#include "Uiana.generated.h"

class FToolBarBuilder;
class FMenuBuilder;
class UUianaDataSettings;

USTRUCT()
struct FTestInner
{
	GENERATED_BODY();
	UPROPERTY()
	FString InnerName;
	UPROPERTY()
	FString OptionalInnerName;
};

USTRUCT()
struct FTestJson
{
	GENERATED_BODY();
	UPROPERTY()
	FString Name;
	UPROPERTY()
	FString Type;
	UPROPERTY()
	FString OptionalStr;
	UPROPERTY()
	FTestInner OptionalInner;
	FTestJson() {};
	// FTestJson(FString _Name, FString _Type)
	// {
	// 	Name = _Name;
	// 	Type = _Type;
	// 	OptionalStr = "";
	// }
};

class FUianaModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** This function will be bound to Command. */
	UFUNCTION()
	FString GetMapName(int EnumValue);
	void PluginButtonClicked();
	FReply ExecuteFunction();
	UPROPERTY(Category = MapsAndSets, EditAnywhere)
	mutable UUianaDataSettings* Stun;

private:
	void RegisterMenus();
	void RegisterSettings();
	TSharedRef<SWidget> MakeDataManagementSettingsDetailsWidget() const;
	void OnTextCommited();
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);


private:
	TWeakObjectPtr<UUianaDataSettings> DataManagementSettingsObjectPtr;
	TSharedPtr<class FUICommandList> PluginCommands;
};
