// Copyright Epic Games, Inc. All Rights Reserved.

#include "Uiana.h"
#include "UianaStyle.h"
#include "Engine/World.h"
#include "UianaCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "ToolMenus.h"
#include "PropertyEditorModule.h"
#include "UianaDataSettings.h"
#include "Misc/Paths.h"
#include "ISettingsModule.h"
static const FName UianaTabName("Uiana");

#define LOCTEXT_NAMESPACE "FUianaModule"

void FUianaModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FUianaStyle::Initialize();
	FUianaStyle::ReloadTextures();
	RegisterSettings();
	FUianaCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FUianaCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FUianaModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUianaModule::RegisterMenus));
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(UianaTabName, FOnSpawnTab::CreateRaw(this, &FUianaModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FeditortestTabTitle", "Valorant Map Importer"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}
void FUianaModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "Uiana",
			LOCTEXT("RuntimeSettingsName", "Uiana"),
			LOCTEXT("RuntimeSettingsDescription", "Configure Uiana settings"),
			GetMutableDefault<UUianaDataSettings>()
		);
	}
}
void FUianaModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FUianaStyle::Shutdown();

	FUianaCommands::Unregister();
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(UianaTabName);
}




void FUianaModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(UianaTabName);
}

FReply FUianaModule::ExecuteFunction()
{
	bool ImportMesh = Stun->ImportMeshes;
	bool ImportMat = Stun->ImportMaterials;
	bool ImportDecal = Stun->ImportDecals;
	bool ImportLights = Stun->ImportLights;
	bool ImportSubLevels = Stun->UseSubLevels;
	bool ImportBlueprint = Stun->ImportBlueprints;
	float ManualLMResMult = Stun->LightmapResolutionMultiplier;
	FString MapName = UEnum::GetValueAsName(Stun->Map).ToString().ToLower();
	//FString MapName = GetMapName(Stun->Map.GetValue());
	FString ExportPath = Stun->ExportFolder.Path;
	FString PakFolder = Stun->PaksFolder.Path;
	FString CurrentPath = FPaths::ProjectPluginsDir();
	Stun->SaveConfig();
	TArray< FStringFormatArg > args;
	args.Add(FStringFormatArg(ImportBlueprint));
	args.Add(FStringFormatArg(ManualLMResMult));
	args.Add(FStringFormatArg(ImportSubLevels));
	args.Add(FStringFormatArg(ImportMesh));
	args.Add(FStringFormatArg(ImportMat));
	args.Add(FStringFormatArg(ImportDecal));
	args.Add(FStringFormatArg(ImportLights));
	args.Add(FStringFormatArg(MapName));
	args.Add(FStringFormatArg(ExportPath));
	args.Add(FStringFormatArg(PakFolder));
	args.Add(FStringFormatArg(CurrentPath));
	FString FormattedConsoleCommand = FString::Format(TEXT("py mods/__init__.py \"{0}\" \"{1}\" \"{2}\" \"{3}\" \"{4}\" \"{5}\" \"{6}\" \"{7}\" \"{8}\" \"{9}\"\"{10}\""), args);
	const TCHAR* TCharCommand = *FormattedConsoleCommand;
	GEngine->Exec(NULL, TCharCommand);
	return FReply::Handled();
}


void FUianaModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FUianaCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FUianaCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

TSharedRef<SWidget> FUianaModule::MakeDataManagementSettingsDetailsWidget() const
{
	UObject* Container = NewObject<UUianaDataSettings>();
	Stun = Cast<UUianaDataSettings>(Container);
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::Get().LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bShowScrollBar = false;
	TSharedRef<IDetailsView> Details = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	PropertyEditorModule.CreatePropertyTable();
	Details->SetObject(Container);
	Details->SetEnabled(true);

	return Details;
}

TSharedRef<class SDockTab> FUianaModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FPLUGIN_NAMEModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("PLUGIN_NAME.cpp"))
	);
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBorder)
			.BorderImage(new FSlateColorBrush(FColor(5, 5, 5)))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.f, 5.f, 8.f, 0.f)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(1.f, 1.f, 0.f, 0.f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)






		+SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Bottom)
		[
			MakeDataManagementSettingsDetailsWidget()
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Right)
		.Padding(2.f, 5.f)
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "FlatButton.Success")
		.ForegroundColor(FSlateColor::UseForeground())
		.OnClicked(FOnClicked::CreateRaw(this, &FUianaModule::ExecuteFunction))
		[
			SNew(STextBlock)
			.Justification(ETextJustify::Center)
		.TextStyle(FAppStyle::Get(), "NormalText.Important")
		.Text(NSLOCTEXT("LevelSnapshots", "NotificationFormatText_CreationForm_CreateSnapshotButton", "Generate Map"))
		]
		]
		]
		]
		]
		];
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUianaModule, Uiana)