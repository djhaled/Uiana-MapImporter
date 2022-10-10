// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "UianaStyle.h"

class FUianaCommands : public TCommands<FUianaCommands>
{
public:

	FUianaCommands()
		: TCommands<FUianaCommands>(TEXT("Uiana"), NSLOCTEXT("Contexts", "Uiana", "Uiana Plugin"), NAME_None, FUianaStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};
