// Copyright Epic Games, Inc. All Rights Reserved.

#include "UianaStyle.h"
#include "Uiana.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FUianaStyle::StyleInstance = nullptr;

void FUianaStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FUianaStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FUianaStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("UianaStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FUianaStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("UianaStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("Uiana")->GetBaseDir() / TEXT("Resources"));

	Style->Set("Uiana.OpenPluginWindow", new IMAGE_BRUSH(TEXT("Icon128"), Icon20x20));
	return Style;
}

void FUianaStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FUianaStyle::Get()
{
	return *StyleInstance;
}
