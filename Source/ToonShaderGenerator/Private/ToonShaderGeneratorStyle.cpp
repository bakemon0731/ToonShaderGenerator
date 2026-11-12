// Copyright Epic Games, Inc. All Rights Reserved.

#include "ToonShaderGeneratorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FToonShaderGeneratorStyle::StyleInstance = nullptr;

void FToonShaderGeneratorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FToonShaderGeneratorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FToonShaderGeneratorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("ToonShaderGeneratorStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FToonShaderGeneratorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("ToonShaderGeneratorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("ToonShaderGenerator")->GetBaseDir() / TEXT("Resources"));

	Style->Set("ToonShaderGenerator.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

	return Style;
}

void FToonShaderGeneratorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FToonShaderGeneratorStyle::Get()
{
	return *StyleInstance;
}
