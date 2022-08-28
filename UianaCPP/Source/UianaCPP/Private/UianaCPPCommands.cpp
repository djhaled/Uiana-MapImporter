// Copyright Epic Games, Inc. All Rights Reserved.

#include "UianaCPPCommands.h"

#define LOCTEXT_NAMESPACE "FUianaCPPModule"

void FUianaCPPCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "UianaCPP", "Generate Valorant Map selected", EUserInterfaceActionType::Button,
	           FInputChord());
}

#undef LOCTEXT_NAMESPACE
