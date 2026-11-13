#include "UI/SToonShaderGeneratorWidget.h"

#include "AssetRegistry/AssetData.h"
#include "Engine/SkeletalMesh.h"
#include "PropertyCustomizationHelpers.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SToonShaderGeneratorWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.Padding(FMargin(20.0f))
		[
			SNew(SVerticalBox)

			// タイトル
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Toon Shader Generator")))
				.Font(FAppStyle::GetFontStyle("HeadingExtraLarge"))
			]

			// 説明
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 16.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(
					TEXT("キャラクター向けトゥーンシェーダーを生成するEditorツールです。")
				))
				.AutoWrapText(true)
			]

			// 生成対象セクション
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("対象メッシュ")))
			]

			// Skeletal Mesh選択欄
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(SObjectPropertyEntryBox)

				//  選択できるアセットを制限
				.AllowedClass(USkeletalMesh::StaticClass())

				// 現在のアセットパス
				.ObjectPath(
					this,
					&SToonShaderGeneratorWidget::GetSelectedSkeletalMeshPath
				)

				// アセット変更時の処理
				.OnObjectChanged(
					this,
					&SToonShaderGeneratorWidget::HandleSkeletalMeshChanged
				)

				// UIオプション
				.AllowClear(true)
				.DisplayThumbnail(true)
				.DisplayBrowse(true)
				.DisplayUseSelected(true)
			]

			// 選択結果
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				SAssignNew(SelectedMeshText, STextBlock)
				.Text(FText::FromString(TEXT("選択中：なし")))
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSeparator)
			]
			
			// 実行結果表示
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 0.0f)
			[
				SAssignNew(StatusText, STextBlock)
				.Text(FText::FromString(TEXT("状態：待機中")))
			]
		]
	];
}

// ==================================================
// Skeletal Mesh変更時の処理
// ==================================================
void SToonShaderGeneratorWidget::HandleSkeletalMeshChanged(const FAssetData& AssetData)
{
	// アセットが選択解除された場合
	if (!AssetData.IsValid())
	{
		SelectedSkeletalMeshPath.Reset();
		
		SelectedMeshText->SetText(
			FText::FromString(TEXT("選択中：なし"))
		);
		
		StatusText->SetText(
			FText::FromString(TEXT("状態：アセットが選択解除されました。"))
		);
		
		return;
	}
	
	// アセットの参照パスを保存
	SelectedSkeletalMeshPath = AssetData.GetSoftObjectPath();
	
	// 選択されたアセット名を取得
	const FString SelectedName = AssetData.AssetName.ToString();
	
	// UIに選択結果を表示
	SelectedMeshText->SetText(
		FText::FromString(
			FString::Printf(TEXT("選択中：%s"), *SelectedName)
		)	
	);
	
	StatusText->SetText(
		FText::FromString(TEXT("状態：Skeletal Meshを選択しました。"))
	);	
}

// ==================================================
// 現在のアセットパスを取得
// ==================================================
FString SToonShaderGeneratorWidget::GetSelectedSkeletalMeshPath() const
{
	return SelectedSkeletalMeshPath.ToString();
}