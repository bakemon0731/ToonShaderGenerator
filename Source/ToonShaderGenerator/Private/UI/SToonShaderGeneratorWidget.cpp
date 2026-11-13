
#include "UI/SToonShaderGeneratorWidget.h"

#include "Core/ToonMeshInspector.h"

#include "AssetRegistry/AssetData.h"
#include "Engine/SkeletalMesh.h"
#include "PropertyCustomizationHelpers.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"


// ==================================================
// Material用途の表示名
// ==================================================

namespace
{
    FText GetMaterialTypeDisplayName(EToonMaterialType MaterialType)
    {
        switch (MaterialType)
        {
        case EToonMaterialType::Face:
            return FText::FromString(TEXT("Face"));

        case EToonMaterialType::Hair:
            return FText::FromString(TEXT("Hair"));

        case EToonMaterialType::Skin:
            return FText::FromString(TEXT("Skin"));

        case EToonMaterialType::Clothing:
            return FText::FromString(TEXT("Clothing"));

        case EToonMaterialType::Other:
        default:
            return FText::FromString(TEXT("Other"));
        }
    }
}


// ==================================================
// UI構築
// ==================================================

void SToonShaderGeneratorWidget::Construct(const FArguments& InArgs)
{
    // 用途の選択肢を初期化
    MaterialTypeOptions.Add(
        MakeShared<EToonMaterialType>(EToonMaterialType::Face)
    );

    MaterialTypeOptions.Add(
        MakeShared<EToonMaterialType>(EToonMaterialType::Hair)
    );

    MaterialTypeOptions.Add(
        MakeShared<EToonMaterialType>(EToonMaterialType::Skin)
    );

    MaterialTypeOptions.Add(
        MakeShared<EToonMaterialType>(EToonMaterialType::Clothing)
    );

    MaterialTypeOptions.Add(
        MakeShared<EToonMaterialType>(EToonMaterialType::Other)
    );

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
                    TEXT("Material Slotごとに用途を設定してください。")
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
                .AllowedClass(USkeletalMesh::StaticClass())
                .ObjectPath(
                    this,
                    &SToonShaderGeneratorWidget::GetSelectedSkeletalMeshPath
                )
                .OnObjectChanged(
                    this,
                    &SToonShaderGeneratorWidget::HandleSkeletalMeshChanged
                )
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

            // Slot数
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 12.0f, 0.0f, 8.0f)
            [
                SAssignNew(MaterialSlotCountText, STextBlock)
                .Text(FText::FromString(TEXT("Material Slot：未取得")))
            ]

            // Slot一覧
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
// Skeletal Mesh選択変更
// ==================================================

void SToonShaderGeneratorWidget::HandleSkeletalMeshChanged(
    const FAssetData& AssetData)
{
    if (!AssetData.IsValid())
    {
        SelectedSkeletalMeshPath.Reset();

        SelectedMeshText->SetText(
            FText::FromString(TEXT("選択中：なし"))
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
    }

    // Meshが変わったら前回の情報を破棄
    CurrentSlotInfos.Reset();

    MaterialSlotListBox->ClearChildren();

    MaterialSlotCountText->SetText(
        FText::FromString(TEXT("Material Slot：未取得"))
    );

    StatusText->SetText(
        FText::FromString(TEXT("状態：対象メッシュが変更されました。"))
    );
}


// ==================================================
// アセットパス取得
// ==================================================

FString SToonShaderGeneratorWidget::GetSelectedSkeletalMeshPath() const
{
    return SelectedSkeletalMeshPath.ToString();
}


// ==================================================
// Mesh情報取得
// ==================================================

FReply SToonShaderGeneratorWidget::HandleInspectMeshClicked()
{
    if (SelectedSkeletalMeshPath.IsNull())
    {
        StatusText->SetText(
            FText::FromString(TEXT("エラー：Skeletal Meshを選択してください。"))
        );

        return FReply::Handled();
    }

    UObject* LoadedObject = SelectedSkeletalMeshPath.TryLoad();

    USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(LoadedObject);

    if (!IsValid(SkeletalMesh))
    {
        StatusText->SetText(
            FText::FromString(TEXT("エラー：Skeletal Meshをロードできませんでした。"))
        );

        return FReply::Handled();
    }

    TArray<FToonMaterialSlotInfo> SlotInfos;
    FString ErrorMessage;

    const bool bSuccess =
        FToonMeshInspector::GetMaterialSlotInfos(
            SkeletalMesh,
            SlotInfos,
            ErrorMessage
        );

    if (!bSuccess)
    {
        StatusText->SetText(
            FText::FromString(
                FString::Printf(TEXT("エラー：%s"), *ErrorMessage)
            )
        );

        return FReply::Handled();
    }

    UpdateMaterialSlotList(SlotInfos);

    StatusText->SetText(
        FText::FromString(TEXT("状態：Material Slot情報を取得しました。"))
    );

    return FReply::Handled();
}


// ==================================================
// Material Slot一覧更新
// ==================================================

void SToonShaderGeneratorWidget::UpdateMaterialSlotList(
    const TArray<FToonMaterialSlotInfo>& SlotInfos)
{
    MaterialSlotListBox->ClearChildren();

    // 取得結果を保持
    CurrentSlotInfos = SlotInfos;

    MaterialSlotCountText->SetText(
        FText::FromString(
            FString::Printf(
                TEXT("Material Slot：%d件"),
                CurrentSlotInfos.Num()
            )
        )
    );

    if (CurrentSlotInfos.IsEmpty())
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

    // 各Slotを表示
    for (const FToonMaterialSlotInfo& SlotInfo : CurrentSlotInfos)
    {
        const int32 SlotIndex = SlotInfo.SlotIndex;

        const FString SlotTitle = FString::Printf(
            TEXT("[%d] %s"),
            SlotIndex,
            *SlotInfo.SlotName.ToString()
        );

        const FString MaterialText = FString::Printf(
            TEXT("Material：%s"),
            *SlotInfo.MaterialName
        );

        // 現在の用途を取得
        const TSharedPtr<EToonMaterialType> InitialOption =
            FindMaterialTypeOption(SlotInfo.MaterialType);

        MaterialSlotListBox->AddSlot()
        .AutoHeight()
        .Padding(0.0f, 0.0f, 0.0f, 8.0f)
        [
            SNew(SBorder)
            .Padding(10.0f)
            [
                SNew(SVerticalBox)

                // Slot名
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 0.0f, 0.0f, 4.0f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(SlotTitle))
                ]

                // 現在のMaterial
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 0.0f, 0.0f, 8.0f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(MaterialText))
                ]

                // 用途ラベル
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 0.0f, 0.0f, 4.0f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("用途")))
                ]

                // 用途選択
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SComboBox<TSharedPtr<EToonMaterialType>>)

                    .OptionsSource(&MaterialTypeOptions)

                    .InitiallySelectedItem(InitialOption)

                    // ドロップダウン内の選択肢
                    .OnGenerateWidget_Lambda(
                        [](TSharedPtr<EToonMaterialType> Option)
                        {
                            const FText DisplayText = Option.IsValid()
                                ? GetMaterialTypeDisplayName(*Option)
                                : FText::FromString(TEXT("Unknown"));

                            return SNew(STextBlock)
                                .Text(DisplayText);
                        }
                    )

                    // 用途変更イベント
                    .OnSelectionChanged_Lambda(
                        [this, SlotIndex](
                            TSharedPtr<EToonMaterialType> SelectedType,
                            ESelectInfo::Type SelectInfo)
                        {
                            HandleMaterialTypeChanged(
                                SelectedType,
                                SlotIndex
                            );
                        }
                    )

                    // 現在選択中の表示
                    [
                        SNew(STextBlock)
                        .Text(
                            this,
                            &SToonShaderGeneratorWidget::GetMaterialTypeText,
                            SlotIndex
                        )
                    ]
                ]
            ]
        ];
    }
}


// ==================================================
// Material用途変更
// ==================================================

void SToonShaderGeneratorWidget::HandleMaterialTypeChanged(
    TSharedPtr<EToonMaterialType> SelectedType,
    int32 SlotIndex)
{
    if (!SelectedType.IsValid())
    {
        return;
    }

    if (!CurrentSlotInfos.IsValidIndex(SlotIndex))
    {
        return;
    }

    // 選択された用途を保存
    CurrentSlotInfos[SlotIndex].MaterialType = *SelectedType;

    const FText TypeName = GetMaterialTypeDisplayName(*SelectedType);

    const FString SlotName =
        CurrentSlotInfos[SlotIndex].SlotName.ToString();

    StatusText->SetText(
        FText::FromString(
            FString::Printf(
                TEXT("%s の用途を %s に設定しました。"),
                *SlotName,
                *TypeName.ToString()
            )
        )
    );
}


// ==================================================
// 現在の用途表示
// ==================================================

FText SToonShaderGeneratorWidget::GetMaterialTypeText(int32 SlotIndex) const
{
    if (!CurrentSlotInfos.IsValidIndex(SlotIndex))
    {
        return FText::FromString(TEXT("Other"));
    }

    return GetMaterialTypeDisplayName(
        CurrentSlotInfos[SlotIndex].MaterialType
    );
}


// ==================================================
// 用途に対応する選択肢を取得
// ==================================================

TSharedPtr<EToonMaterialType>
SToonShaderGeneratorWidget::FindMaterialTypeOption(
    EToonMaterialType MaterialType) const
{
    for (const TSharedPtr<EToonMaterialType>& Option : MaterialTypeOptions)
    {
        if (Option.IsValid() && *Option == MaterialType)
        {
            return Option;
        }
    }

    return nullptr;
}
