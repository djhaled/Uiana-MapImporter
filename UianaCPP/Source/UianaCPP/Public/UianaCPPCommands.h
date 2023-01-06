// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "UianaCPPStyle.h"

class FUianaCPPCommands : public TCommands<FUianaCPPCommands>
{
public:
	FUianaCPPCommands()
		: TCommands<FUianaCPPCommands>(TEXT("UianaCPP"), NSLOCTEXT("Contexts", "UianaCPP", "UianaCPP Plugin"), NAME_None,
		                            FUianaCPPStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;
	
	TSharedPtr<FUICommandInfo> OpenPluginWindow;
};
