// Copyright Epic Games, Inc. All Rights Reserved.

#include "UianaCPPStyle.h"
#include "UianaCPP.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FUianaCPPStyle::StyleInstance = nullptr;

void FUianaCPPStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FUianaCPPStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FUianaCPPStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("UianaCPPStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef<FSlateStyleSet> FUianaCPPStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("UianaCPPStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("UianaCPP")->GetBaseDir() / TEXT("Resources"));

	Style->Set("UianaCPP.OpenPluginWindow", new IMAGE_BRUSH(TEXT("Icon128"), Icon20x20));
	return Style;
}

void FUianaCPPStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FUianaCPPStyle::Get()
{
	return *StyleInstance;
}
