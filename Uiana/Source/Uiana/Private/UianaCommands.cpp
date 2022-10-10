// Copyright Epic Games, Inc. All Rights Reserved.

#include "UianaCommands.h"

#define LOCTEXT_NAMESPACE "FUianaModule"

void FUianaCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Uiana", "Generate Valorant Map selected", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
