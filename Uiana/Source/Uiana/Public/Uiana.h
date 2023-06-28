// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;
class UUianaDataSettings;

class FUianaModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	UFUNCTION()
	void PluginButtonClicked();
	FReply ExecuteFunction();
	UPROPERTY(Category = MapsAndSets, EditAnywhere)
	mutable UUianaDataSettings*  Stun;

	
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
