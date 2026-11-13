#include "UI/SToonShaderGeneratorWidget.h"

#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
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
				.Text(FText::FromString(TEXT("生成対象")))
				.Font(FAppStyle::GetFontStyle("HeadingMedium"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 16.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(
					TEXT("次のステップでSkeletal Meshとテクスチャの選択機能を追加します。")
				))
				.AutoWrapText(true)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSeparator)
			]

			// 動作確認セクション
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 8.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("動作確認")))
				.Font(FAppStyle::GetFontStyle("HeadingMedium"))
			]

			// テストボタン
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("UIイベントをテスト")))
				.OnClicked(
					this,
					&SToonShaderGeneratorWidget::HandleTestButtonClicked
				)	
			]

			// 実行結果表示
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(StatusText, STextBlock)
				.Text(FText::FromString(TEXT("状態：待機中")))
			]
		]
	];
}

FReply SToonShaderGeneratorWidget::HandleTestButtonClicked()
{
	if (StatusText.IsValid())
	{
		StatusText->SetText(
			FText::FromString(TEXT("状態：ボタンイベントが正常に動作しました。"))
		);	
	}
	return FReply::Handled();
}


