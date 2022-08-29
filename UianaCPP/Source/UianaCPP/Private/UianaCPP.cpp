// Copyright Epic Games, Inc. All Rights Reserved.

#include "UianaCPP.h"

#include "BPFL.h"
#include "UianaCPPStyle.h"
#include "Engine/World.h"
#include "UianaCPPCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "ToolMenus.h"
#include "PropertyEditorModule.h"
#include "UianaCPPDataSettings.h"
#include "Misc/Paths.h"
#include "ISettingsModule.h"
static const FName UianaCPPTabName("UianaCPP");

#define LOCTEXT_NAMESPACE "FUianaCPPModule"

void FUianaCPPModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FUianaCPPStyle::Initialize();
	FUianaCPPStyle::ReloadTextures();
	RegisterSettings();
	FUianaCPPCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FUianaCPPCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FUianaCPPModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUianaCPPModule::RegisterMenus));
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(UianaCPPTabName,
	                                                  FOnSpawnTab::CreateRaw(this, &FUianaCPPModule::OnSpawnPluginTab))
	                        .SetDisplayName(LOCTEXT("FeditortestTabTitle", "Valorant Map Importer"))
	                        .SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FUianaCPPModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "UianaCPP",
		                                 LOCTEXT("RuntimeSettingsName", "UianaCPP"),
		                                 LOCTEXT("RuntimeSettingsDescription", "Configure UianaCPP settings"),
		                                 GetMutableDefault<UUianaCPPDataSettings>()
		);
	}
}

void FUianaCPPModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FUianaCPPStyle::Shutdown();

	FUianaCPPCommands::Unregister();
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(UianaCPPTabName);
}


FString FUianaCPPModule::GetMapName(int EnumValue)
{
	if (EnumValue == 0)
	{
		return FString("ascent");
	}
	if (EnumValue == 1)
	{
		return FString("split");
	}
	if (EnumValue == 2)
	{
		return FString("bind");
	}
	if (EnumValue == 3)
	{
		return FString("icebox");
	}
	if (EnumValue == 4)
	{
		return FString("breeze");
	}
	if (EnumValue == 5)
	{
		return FString("haven");
	}
	if (EnumValue == 6)
	{
		return FString("fracture");
	}
	if (EnumValue == 7)
	{
		return FString("range");
	}
	if (EnumValue == 8)
	{
		return FString("pearl");
	}
	if (EnumValue == 9)
	{
		return FString("characterSelect");
	}
	if (EnumValue == 10)
	{
		return FString("menu");
	}
	return FString("NoMap");
}

void FUianaCPPModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(UianaCPPTabName);
}

FReply FUianaCPPModule::ExecuteFunction()
{
	bool ImportMesh = Stun->ImportMeshes;
	bool ImportMat = Stun->ImportMaterials;
	bool ImportDecal = Stun->ImportDecals;
	bool ImportLights = Stun->ImportLights;
	bool ImportSubLevels = Stun->UseSubLevels;
	FString MapName = GetMapName(Stun->Map.GetValue());
	FString ExportPath = Stun->ExportFolder.Path;
	FString PakFolder = Stun->PaksFolder.Path;
	FString CurrentPath = FPaths::ProjectPluginsDir();
	Stun->SaveConfig();
	
	TArray<FStringFormatArg> args;
	args.Add(FStringFormatArg(ImportSubLevels));
	args.Add(FStringFormatArg(ImportMesh));
	args.Add(FStringFormatArg(ImportMat));
	args.Add(FStringFormatArg(ImportDecal));
	args.Add(FStringFormatArg(ImportLights));
	args.Add(FStringFormatArg(MapName));
	args.Add(FStringFormatArg(ExportPath));
	args.Add(FStringFormatArg(PakFolder));
	args.Add(FStringFormatArg(CurrentPath));
	FString FormattedConsoleCommand = FString::Format(
		TEXT("py mods/__init__.py \"{0}\" \"{1}\" \"{2}\" \"{3}\" \"{4}\" \"{5}\" \"{6}\" \"{7}\" \"{8}\""), args);
	const TCHAR* TCharCommand = *FormattedConsoleCommand;
	GEngine->Exec(nullptr, TCharCommand);
	return FReply::Handled();
}

void FUianaCPPModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FUianaCPPCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(
					FToolMenuEntry::InitToolBarButton(FUianaCPPCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

TSharedRef<SWidget> FUianaCPPModule::MakeDataManagementSettingsDetailsWidget() const
{
	UObject* Container = NewObject<UUianaCPPDataSettings>();
	Stun = Cast<UUianaCPPDataSettings>(Container);
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::Get().LoadModuleChecked<FPropertyEditorModule>(
		"PropertyEditor");
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

TSharedRef<class SDockTab> FUianaCPPModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FPLUGIN_NAMEModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("PLUGIN_NAME.cpp"))
	);
	return SNew(SDockTab)
		.TabRole(NomadTab)
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
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						  .AutoHeight()
						  .Padding(1.f, 1.f, 0.f, 0.f)
						  .HAlign(HAlign_Left)
						  .VAlign(VAlign_Top)


						+ SVerticalBox::Slot()
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
			.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
		.ForegroundColor(FSlateColor::UseForeground())
		.OnClicked(FOnClicked::CreateRaw(this, &FUianaCPPModule::ExecuteFunction))
							[
								SNew(STextBlock)
			.Justification(ETextJustify::Center)
		.TextStyle(FEditorStyle::Get(), "NormalText.Important")
		.Text(NSLOCTEXT("LevelSnapshots", "NotificationFormatText_CreationForm_CreateSnapshotButton", "Generate Map"))
							]
						]
					]
				]
			]
		];
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUianaCPPModule, UianaCPP)
