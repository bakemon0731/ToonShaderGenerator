// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "ToonShaderGeneratorStyle.h"

class FToonShaderGeneratorCommands : public TCommands<FToonShaderGeneratorCommands>
{
public:

	FToonShaderGeneratorCommands()
		: TCommands<FToonShaderGeneratorCommands>(TEXT("ToonShaderGenerator"), NSLOCTEXT("Contexts", "ToonShaderGenerator", "ToonShaderGenerator Plugin"), NAME_None, FToonShaderGeneratorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};