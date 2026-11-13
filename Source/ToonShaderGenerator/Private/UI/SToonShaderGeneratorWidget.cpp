#include "UI/SToonShaderGeneratorWidget.h"

#include "Core/ToonMeshInspector.h"

#include "AssetRegistry/AssetData.h"
#include "Engine/SkeletalMesh.h"
#include "PropertyCustomizationHelpers.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScaleBox.h"
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
			]

			// 対象メッシュ
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("対象メッシュ")))
			]

			// Mesh選択欄
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

			// 選択中のMesh
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				SAssignNew(SelectedMeshText, STextBlock)
				.Text(FText::FromString(TEXT("選択中：なし")))
			]

			// 情報取得ボタン
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Material Slot情報を取得")))
				.OnClicked(
					this,
					&SToonShaderGeneratorWidget::HandleInspectMeshClicked
				)	
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSeparator)
			]

			// Material Slot数
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 8.0f)
			[
				SAssignNew(MaterialSlotCountText,STextBlock)
				.Text(FText::FromString(TEXT("Material Slot：未取得")))
			]

			// Material Slot一覧
			+ SVerticalBox::Slot()
		   .FillHeight(1.0f)
		   [
			   SNew(SScrollBox)
			   + SScrollBox::Slot()
			   [
				   SAssignNew(MaterialSlotListBox, SVerticalBox)
			   ]
		   ]
			
			// 状態表示
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
	}
	else
	{
		SelectedSkeletalMeshPath = AssetData.GetSoftObjectPath();
		
		const FString SelectedName = AssetData.AssetName.ToString();
		
		SelectedMeshText->SetText(
			FText::FromString(
				FString::Printf(TEXT("選択中：%s"), *SelectedName)
			)
		);
		
		StatusText->SetText(
			FText::FromString(TEXT("状態：Skeletal Meshを選択しました。"))
		);	
	}
	
	// Mesh変更時は以前の取得結果を消す
	MaterialSlotListBox->ClearChildren();
	
	MaterialSlotCountText->SetText(
		FText::FromString(TEXT("Material Slot：未取得"))
	);	
}

// ==================================================
// 現在のアセットパスを取得
// ==================================================
FString SToonShaderGeneratorWidget::GetSelectedSkeletalMeshPath() const
{
	return SelectedSkeletalMeshPath.ToString();
}

// ==================================================
// Mesh情報取得ボタン
// ==================================================
FReply SToonShaderGeneratorWidget::HandleInspectMeshClicked()
{
	// アセットが選択されているか確認
	if (SelectedSkeletalMeshPath.IsNull())
	{
		StatusText->SetText(
			FText::FromString(TEXT("エラー：Skeletal Meshを選択してください。"))
		);
		return FReply::Handled();
	}
	
	// アセットをロード
	UObject* LoadedObject = SelectedSkeletalMeshPath.TryLoad();
	
	// USkeletalMeshに変換
	USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(LoadedObject);
	
	if (!IsValid(SkeletalMesh))
	{
		StatusText->SetText(
			FText::FromString(TEXT("エラー：Skeletal Meshをロードできませんでした。"))
		);
		return FReply::Handled();
	}
	
	// Material Slot情報を取得
	TArray<FToonMaterialSlotInfo> SlotInfos;
	FString ErrorMessage;
	
	const bool bSuccess = FToonMeshInspector::GetMaterialSlotInfos(SkeletalMesh, SlotInfos, ErrorMessage);

	if (!bSuccess)
	{
		StatusText->SetText(
			FText::FromString(
				FString::Printf(TEXT("エラー：%s"), *ErrorMessage)
			)
		);
		return FReply::Handled();
	}
	
	// UIに取得結果を反映
	UpdateMaterialSlotList(SlotInfos);
	
	StatusText->SetText(
		FText::FromString(TEXT("状態：Material Slot情報の取得に成功しました。"))
	);
	return FReply::Handled();
}

// ==================================================
// Material Slot一覧の表示更新
// ==================================================
void SToonShaderGeneratorWidget::UpdateMaterialSlotList(const TArray<FToonMaterialSlotInfo>& SlotInfos)
{
	MaterialSlotListBox->ClearChildren();
	
	// Slot数を表示
	MaterialSlotCountText->SetText(
		FText::FromString(
			FString::Printf(
				TEXT("Material Slot：%d件"),
				SlotInfos.Num()
			)
		)
	);
	
	if (SlotInfos.Num() == 0)
	{
		MaterialSlotListBox->AddSlot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Material Slotがありません。")))
		];
		return;
	}
	
	// 各Slotの情報を表示
	for (const FToonMaterialSlotInfo& SlotInfo : SlotInfos)
	{
		const FString SlotTitle = FString::Printf(
			TEXT("[%d] %s"),
			SlotInfo.SlotIndex,
			*SlotInfo.SlotName.ToString()
		);	
		
		const FString MaterialText = FString::Printf(
			TEXT("Material：%s"),
			*SlotInfo.MaterialName
		);
		
		MaterialSlotListBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 6.0f)
		[
			SNew(SBorder)
			.Padding(10.0f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 4.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(SlotTitle))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(MaterialText))
				]
			]
		];
	}
}