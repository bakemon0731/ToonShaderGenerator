// Copyright Epic Games, Inc. All Rights Reserved.

#include "ToonShaderGeneratorCommands.h"

#define LOCTEXT_NAMESPACE "FToonShaderGeneratorModule"

void FToonShaderGeneratorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "ToonShaderGenerator", "Bring up ToonShaderGenerator window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
