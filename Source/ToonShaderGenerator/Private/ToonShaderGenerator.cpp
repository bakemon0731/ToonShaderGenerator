// Copyright Epic Games, Inc. All Rights Reserved.

#include "ToonShaderGenerator.h"
#include "ToonShaderGeneratorStyle.h"
#include "ToonShaderGeneratorCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

static const FName ToonShaderGeneratorTabName("ToonShaderGenerator");

#define LOCTEXT_NAMESPACE "FToonShaderGeneratorModule"

void FToonShaderGeneratorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FToonShaderGeneratorStyle::Initialize();
	FToonShaderGeneratorStyle::ReloadTextures();

	FToonShaderGeneratorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FToonShaderGeneratorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FToonShaderGeneratorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FToonShaderGeneratorModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ToonShaderGeneratorTabName, FOnSpawnTab::CreateRaw(this, &FToonShaderGeneratorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FToonShaderGeneratorTabTitle", "ToonShaderGenerator"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FToonShaderGeneratorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FToonShaderGeneratorStyle::Shutdown();

	FToonShaderGeneratorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ToonShaderGeneratorTabName);
}

TSharedRef<SDockTab> FToonShaderGeneratorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FToonShaderGeneratorModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("ToonShaderGenerator.cpp"))
		);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(WidgetText)
			]
		];
}

void FToonShaderGeneratorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ToonShaderGeneratorTabName);
}

void FToonShaderGeneratorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FToonShaderGeneratorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FToonShaderGeneratorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FToonShaderGeneratorModule, ToonShaderGenerator)