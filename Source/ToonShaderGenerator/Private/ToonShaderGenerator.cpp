// Copyright Epic Games, Inc. All Rights Reserved.

#include "ToonShaderGenerator.h"
#include "ToonShaderGeneratorStyle.h"
#include "ToonShaderGeneratorCommands.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "UI/SToonShaderGeneratorWidget.h"

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

// Module側はウィンドウを開く責任だけを持ち、UIの構築は別クラスが担当
TSharedRef<SDockTab> FToonShaderGeneratorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SToonShaderGeneratorWidget)
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