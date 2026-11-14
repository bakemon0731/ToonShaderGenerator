
#include "UI/SToonShaderGeneratorWidget.h"

#include "Core/ToonMeshInspector.h"
#include "Core/ToonMaterialInstanceCreator.h"
#include "Core/ToonMeshMaterialAssignmentService.h"
#include "Core/ToonTextureValidator.h"

#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInterface.h"

#include "AssetRegistry/AssetData.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Texture2D.h"
#include "PropertyCustomizationHelpers.h"
#include "Templates/SharedPointer.h"
#include "Misc/MessageDialog.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Colors/SColorPicker.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/SBoxPanel.h"


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
    // プラグイン標準のMaster Material
    SelectedMasterMaterialPath = FSoftObjectPath(
        TEXT("/ToonShaderGenerator/Materials/Master/M_Toon_Basic.M_Toon_Basic"));
    
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

            // ==========================================
            // Master Material選択
            // ==========================================

            //ラベル
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 0.0f, 0.0f, 6.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Master Material")))
            ]
            
            // Master Materialの情報表示
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 0.0f, 0.0f, 16.0f)
            [
                SNew(SBorder)
                .Padding(10.0f)
                [
                    SAssignNew(MasterMaterialText, STextBlock)
                    .Text(FText::FromString(
                        TEXT("Master Material：M_Toon_Basic（プラグイン標準）")
                    ))    
                ]
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

            // ==========================================
            // 基本Toon Shader設定
            // ==========================================

            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 12.0f, 0.0f, 8.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("基本シェーダー設定")))
            ]

            // Base Color
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 4.0f)
            [
                SNew(SButton)
                .OnClicked(
                    this,
                    &SToonShaderGeneratorWidget::HandleOpenColorPicker,
                    true
                )
                [
                    SNew(SHorizontalBox)

                    + SHorizontalBox::Slot()
                    .FillWidth(1.0f)
                    .VAlign(VAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Base Color")))
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SColorBlock)
                        .Color_Lambda(
                            [this]()
                            {
                                return BasicMaterialSettings.BaseColor;
                            }
                        )
                        .Size(FVector2D(100.0f, 24.0f))
                        .ShowBackgroundForAlpha(false)
                    ]
                ]
            ]

            // Shadow Color
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 4.0f)
            [
                SNew(SButton)
                .OnClicked(
                    this,
                    &SToonShaderGeneratorWidget::HandleOpenColorPicker,
                    false
                )
                [
                    SNew(SHorizontalBox)

                    + SHorizontalBox::Slot()
                    .FillWidth(1.0f)
                    .VAlign(VAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Shadow Color")))
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SColorBlock)
                        .Color_Lambda(
                            [this]()
                            {
                                return BasicMaterialSettings.ShadowColor;
                            }
                        )
                        .Size(FVector2D(100.0f, 24.0f))
                        .ShowBackgroundForAlpha(false)
                    ]
                ]
            ]

            // Shadow Thresholdのラベル
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 8.0f, 0.0f, 4.0f)
            [
                SNew(STextBlock)
                .Text_Lambda(
                    [this]()
                    {
                        return FText::FromString(
                            FString::Printf(
                                TEXT("Shadow Threshold：%.2f"),
                                BasicMaterialSettings.ShadowThreshold
                            )
                        );
                    }
                )    
            ]

            // Shadow Thresholdのスライダー
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 0.0f, 0.0f, 12.0f)
            [
                SNew(SSlider)
                .MinValue(0.0f)
                .MaxValue(1.0f)
                .Value_Lambda(
                    [this]()
                    {
                        return BasicMaterialSettings.ShadowThreshold;
                    }
                )
                .OnValueChanged(
                    this,
                    &SToonShaderGeneratorWidget::HandleShadowThresholdChanged
                )    
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

            // ==========================================
            // Texture Validator
            // ==========================================

            // 検証ボタン
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 12.0f, 0.0f, 8.0f)
            [
                SNew(SButton)
                .Text(
                    FText::FromString(
                        TEXT("テクスチャを検証")
                    )
                )
                .OnClicked(
                    this,
                    &SToonShaderGeneratorWidget::HandleValidateTexturesClicked
                )    
            ]

            // 検証結果の概要
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 0.0f, 0.0f, 6.0f)
            [
                SAssignNew(ValidationSummaryText, STextBlock)
                .Text(
                    FText::FromString(
                        TEXT("検証：未実行")
                    )
                )    
            ]

            // 検証結果の一覧
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 0.0f, 0.0f, 12.0f)
            [
                SNew(SBorder)
                .Padding(8.0f)
                [
                    SAssignNew(ValidationResultBox, SVerticalBox)
                ]
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
    MaterialInputBoxes.Reset();
    MaterialNameTextBlocks.Reset();

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

    // 同じMeshを再検証した場合は、既存のバックアップと
    // 生成済みMIの参照を引き継ぐ
    for (FToonMaterialSlotInfo& NewInfo : SlotInfos)
    {
        if (!CurrentSlotInfos.IsValidIndex(NewInfo.SlotIndex))
        {
            continue;
        }
        
        const FToonMaterialSlotInfo& PreviousInfo = CurrentSlotInfos[NewInfo.SlotIndex];
        
        if (PreviousInfo.bHasOriginalMaterialBackup)
        {
            NewInfo.OriginalMaterialPath = PreviousInfo.OriginalMaterialPath;
            
            NewInfo.bHasOriginalMaterialBackup = true;
        }
        
        if (!PreviousInfo.GeneratedMaterialInstancePath.IsNull())
        {
            NewInfo.GeneratedMaterialInstancePath = PreviousInfo.GeneratedMaterialInstancePath;
        }
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
    MaterialInputBoxes.Reset();
    MaterialNameTextBlocks.Reset();
    // 前回の検証結果をリセット
    ValidationResultBox->ClearChildren();
    ValidationSummaryText->SetText(FText::FromString(TEXT("検証：未実行")));

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

        const TSharedPtr<EToonMaterialType> InitialOption =
            FindMaterialTypeOption(SlotInfo.MaterialType);

        // このSlot専用のテクスチャ入力領域
        TSharedPtr<SVerticalBox> MaterialInputBox;
        
        TSharedPtr<STextBlock> MaterialNameText;
        
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
                    SAssignNew(MaterialNameText, STextBlock)
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
                .Padding(0.0f, 0.0f, 0.0f, 8.0f)
                [
                    SNew(SComboBox<TSharedPtr<EToonMaterialType>>)
                    .OptionsSource(&MaterialTypeOptions)
                    .InitiallySelectedItem(InitialOption)

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

                    [
                        SNew(STextBlock)
                        .Text(
                            this,
                            &SToonShaderGeneratorWidget::GetMaterialTypeText,
                            SlotIndex
                        )
                    ]
                ]

                // 用途に応じたテクスチャ入力領域
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SAssignNew(MaterialInputBox, SVerticalBox)
                ]

                // Material Instance生成ボタン
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 8.0f, 0.0f, 0.0f)
                [
                    SNew(SButton)
                    .Text(
                        FText::FromString(
                            TEXT("このSlotのMaterial Instanceを生成")
                        )
                    )
                    .OnClicked_Lambda(
                        [this, SlotIndex]()
                        {
                            return HandleGenerateMaterialInstanceClicked(
                                SlotIndex
                            );    
                        }
                    )    
                ]

                // ==========================================
                // 生成したMIを適用
                // ==========================================
                +SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 6.0f, 0.0f, 0.0f)
                [
                    SNew(SButton)
                    .Text(
                        FText::FromString(
                            TEXT("生成したMIをこのSlotに適用")
                        )
                    )
                    .OnClicked_Lambda(
                        [this, SlotIndex]()
                        {
                            return HandleApplyMaterialInstanceClicked(
                                SlotIndex
                            );    
                        }
                    )    
                ]

                // ==========================================
                // 元のMaterialに戻す
                // ==========================================
                +SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 4.0f, 0.0f, 0.0f)
                [
                    SNew(SButton)
                    .Text(
                        FText::FromString(
                            TEXT("元のMaterialに戻す")
                        )
                    )
                    .OnClicked_Lambda(
                        [this, SlotIndex]()
                        {
                            return HandleRestoreOriginalMaterialClicked( SlotIndex );
                        }
                    )    
                ]

            ]
        ];
        
        //SlotごとのMaterial名表示を後から更新
        MaterialNameTextBlocks.Add(SlotIndex, MaterialNameText);

        // Slotと入力領域を関連付ける
        // 例えばSlot 0のFaceの入力欄だけを更新できるようになる
        MaterialInputBoxes.Add(SlotIndex, MaterialInputBox);

        // 現在の用途に応じた入力欄を生成
        RefreshMaterialInputWidgets(SlotIndex);
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
    
    // 用途変更時にUIを更新
    RefreshMaterialInputWidgets(SlotIndex);

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
    
    // 用途変更によって検証対象が変化したためリセット
    ValidationResultBox->ClearChildren();
    ValidationSummaryText->SetText(FText::FromString(TEXT("検証：未実行（用途変更）")));
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

// ==================================================
// テクスチャ入力欄を生成
// ==================================================
TSharedRef<SWidget> SToonShaderGeneratorWidget::CreateTextureInputWidget(int32 SlotIndex,
    EToonTextureInputType InputType, const FText& Label)
{
    return SNew(SVerticalBox)

         // 入力欄のタイトル
         + SVerticalBox::Slot()
         .AutoHeight()
         .Padding(0.0f, 4.0f, 0.0f, 4.0f)
         [
             SNew(STextBlock)
             .Text(Label)
         ]

         // Textureアセット選択欄
         + SVerticalBox::Slot()
         .AutoHeight()
         .Padding(0.0f, 0.0f, 0.0f, 8.0f)
         [
             SNew(SObjectPropertyEntryBox)

             // Texture2Dだけを選択可能にする
             .AllowedClass(UTexture2D::StaticClass())

             // 現在のテクスチャパス
             .ObjectPath(
                 TAttribute<FString>::CreateLambda(
                     [this, SlotIndex, InputType]()
                     {
                         return GetTexturePath(SlotIndex, InputType);
                     }
                 )
             )

             // 選択変更時
             .OnObjectChanged(
                 FOnSetObject::CreateLambda(
                     [this, SlotIndex, InputType](
                         const FAssetData& AssetData)
                     {
                         HandleTextureChanged(
                             AssetData,
                             SlotIndex,
                             InputType
                         );
                     }
                 )
             )

             .AllowClear(true)
             .DisplayThumbnail(true)
             .DisplayBrowse(true)
             .DisplayUseSelected(true)
         ];
}

// ==================================================
// 保存しているテクスチャの参照を返す関数
// ==================================================
FString SToonShaderGeneratorWidget::GetTexturePath(int32 SlotIndex, EToonTextureInputType InputType) const
{
    if (!CurrentSlotInfos.IsValidIndex(SlotIndex))
    {
        return FString();
    }
    
    const FToonMaterialSlotInfo& SlotInfo = CurrentSlotInfos[SlotIndex];

    switch (InputType)
    {
    case EToonTextureInputType::FaceSDF:
        return SlotInfo.FaceSDFTexturePath.ToString();

    case EToonTextureInputType::HairFlowMap:
        return SlotInfo.HairFlowMapTexturePath.ToString();

    case EToonTextureInputType::SkinBaseColor:
        return SlotInfo.SkinBaseColorTexturePath.ToString();

    case EToonTextureInputType::ClothingBaseColor:
        return SlotInfo.ClothingBaseColorTexturePath.ToString();

    case EToonTextureInputType::ClothingNormal:
        return SlotInfo.ClothingNormalTexturePath.ToString();

    case EToonTextureInputType::ClothingORM:
        return SlotInfo.ClothingORMTexturePath.ToString();

    default:
        return FString();
        
        
    }
}

// ==================================================
// 選択されたTextureの参照を保存する関数
// ==================================================
void SToonShaderGeneratorWidget::HandleTextureChanged(const FAssetData& AssetData, int32 SlotIndex,
    EToonTextureInputType InputType)
{
    if (!CurrentSlotInfos.IsValidIndex(SlotIndex))
    {
        return;
    }
    
    // 配列の中にあるデータを参照で取得
    FToonMaterialSlotInfo& SlotInfo = CurrentSlotInfos[SlotIndex];

    // 選択されたTextureの参照パス
    // 選択解除なら空のパスになる
    const FSoftObjectPath TexturePath =
        AssetData.IsValid()
        ? AssetData.GetSoftObjectPath()
        : FSoftObjectPath();

    // 入力種類に応じて保存先を切り替える
    switch (InputType)
    {
    case EToonTextureInputType::FaceSDF:
        SlotInfo.FaceSDFTexturePath = TexturePath;
        break;

    case EToonTextureInputType::HairFlowMap:
        SlotInfo.HairFlowMapTexturePath = TexturePath;
        break;

    case EToonTextureInputType::SkinBaseColor:
        SlotInfo.SkinBaseColorTexturePath = TexturePath;
        break;

    case EToonTextureInputType::ClothingBaseColor:
        SlotInfo.ClothingBaseColorTexturePath = TexturePath;
        break;

    case EToonTextureInputType::ClothingNormal:
        SlotInfo.ClothingNormalTexturePath = TexturePath;
        break;

    case EToonTextureInputType::ClothingORM:
        SlotInfo.ClothingORMTexturePath = TexturePath;
        break;

    default:
        return;
    }

    StatusText->SetText(
        FText::FromString(
            AssetData.IsValid()
            ? TEXT("状態：テクスチャを設定しました。")
            : TEXT("状態：テクスチャを解除しました。")
        )
    );
    
    // テクスチャを変更したため検証結果を無効化
    ValidationResultBox->ClearChildren();
    ValidationSummaryText->SetText(FText::FromString(TEXT("検証：未実行（テクスチャ変更）")));
}

// ==================================================
// 用途に応じて入力欄を切り替える関数
// ==================================================
void SToonShaderGeneratorWidget::RefreshMaterialInputWidgets(int32 SlotIndex)
{
    if (!CurrentSlotInfos.IsValidIndex(SlotIndex))
    {
        return;
    }

    TSharedPtr<SVerticalBox>* FoundBox = MaterialInputBoxes.Find(SlotIndex);

    if (FoundBox == nullptr || !FoundBox->IsValid())
    {
        return;
    }

    TSharedPtr<SVerticalBox> InputBox = *FoundBox;

    // 以前の入力欄を削除
    InputBox->ClearChildren();

    const EToonMaterialType MaterialType =
        CurrentSlotInfos[SlotIndex].MaterialType;

    // 共通の入力欄追加処理
    auto AddTextureInput =
        [this, SlotIndex, InputBox](
            EToonTextureInputType InputType,
            const FText& Label)
        {
            InputBox->AddSlot()
            .AutoHeight()
            .Padding(0.0f, 0.0f, 0.0f, 4.0f)
            [
                CreateTextureInputWidget(
                    SlotIndex,
                    InputType,
                    Label
                )
            ];
        };
    
    // Material Slotの用途を確認
    switch (MaterialType)
    {
    case EToonMaterialType::Face:

        AddTextureInput(
            EToonTextureInputType::FaceSDF,
            FText::FromString(TEXT("Face SDF Texture"))
        );

        break;

    case EToonMaterialType::Hair:

        AddTextureInput(
            EToonTextureInputType::HairFlowMap,
            FText::FromString(TEXT("Hair Flow Map"))
        );

        break;

    case EToonMaterialType::Skin:

        AddTextureInput(
            EToonTextureInputType::SkinBaseColor,
            FText::FromString(TEXT("Skin Base Color"))
        );

        break;

    case EToonMaterialType::Clothing:

        AddTextureInput(
            EToonTextureInputType::ClothingBaseColor,
            FText::FromString(TEXT("Base Color"))
        );

        AddTextureInput(
            EToonTextureInputType::ClothingNormal,
            FText::FromString(TEXT("Normal"))
        );

        AddTextureInput(
            EToonTextureInputType::ClothingORM,
            FText::FromString(TEXT("ORM"))
        );

        break;

    case EToonMaterialType::Other:

    default:

        InputBox->AddSlot()
        .AutoHeight()
        [
            SNew(STextBlock)
            .Text(FText::FromString(
                TEXT("追加テクスチャ入力はありません。")
            ))
        ];

        break;
    }
}

// ==================================================
// Material Instance生成関数
// ==================================================
FReply SToonShaderGeneratorWidget::HandleGenerateMaterialInstanceClicked(int32 SlotIndex)
{
    
    // Slotの有効性を確認
    if (!CurrentSlotInfos.IsValidIndex(SlotIndex))
    {
        StatusText->SetText(
            FText::FromString(
                TEXT("エラー：Material Slotが無効です。")
            )
        );
        return FReply::Handled();
    }
    
    // Master Materialを確認
    if (SelectedMasterMaterialPath.IsNull())
    {
        StatusText->SetText(
            FText::FromString(
                TEXT("エラー：Master Materialを選択してください。")
            )
        );
        return FReply::Handled();
    }
    
    UObject* LoadedMasterObject = SelectedMasterMaterialPath.TryLoad();
    
    UMaterial* MasterMaterial = Cast<UMaterial>(LoadedMasterObject);
    
    if (!IsValid(MasterMaterial))
    {
        StatusText->SetText(
            FText::FromString(
                TEXT("エラー：プラグイン標準のM_Toon_Basicを読み込めません。Plugin Contentの配置を確認してください。")
            )
        );
        return FReply::Handled();
    }
    
    // Skeletal Meshを確認
    UObject* LoadedMeshObject = SelectedSkeletalMeshPath.TryLoad();
    
    USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(LoadedMeshObject);
    
    if (!IsValid(SkeletalMesh))
    {
        StatusText->SetText(
            FText::FromString(
                TEXT("エラー：Skeletal Meshを読み込めません。")
            )
        );
        return FReply::Handled();
    }
    
    // 生成するアセット名を決定
    const FString AssetName = FString::Printf(
        TEXT("MI_Toon_%s_Slot_%02d"),
        *SkeletalMesh->GetName(),
        SlotIndex
    );
    
    // 生成先
    const FString PackagePath = TEXT("/Game/ToonGenerated/Materials");
    
    // Material Instanceを生成
    FString ErrorMessage;
    
    UMaterialInstanceConstant* CreatedInstance =
        FToonMaterialInstanceCreator::Create(
            MasterMaterial,
            AssetName,
            PackagePath,
            BasicMaterialSettings,
            ErrorMessage
        );
    
    if (!IsValid(CreatedInstance))
    {
        StatusText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("生成失敗：%s"),
                    * ErrorMessage
                )
            )
        );
        return FReply::Handled();
    }
    
    // 生成したMIの参照をSlot情報に保存
    CurrentSlotInfos[SlotIndex].GeneratedMaterialInstancePath =
        FSoftObjectPath(CreatedInstance->GetPathName());
    
    // 結果を表示
    StatusText->SetText(
        FText::FromString(
            FString::Printf(
                TEXT("生成成功：%s"),
                *CreatedInstance->GetName()
            )
        )    
    );
    return FReply::Handled();
}

FReply SToonShaderGeneratorWidget::HandleOpenColorPicker(bool bEditBaseColor)
{
    // 編集対象の色を取得
    const FLinearColor InitialColor = bEditBaseColor
        ? BasicMaterialSettings.BaseColor
        : BasicMaterialSettings.ShadowColor;

    // カラーピッカーの設定
    FColorPickerArgs PickerArgs(
        InitialColor,
        FOnLinearColorValueChanged::CreateLambda(
            [this, bEditBaseColor](FLinearColor NewColor)
            {
                // 選択された色を保存
                if (bEditBaseColor)
                {
                    BasicMaterialSettings.BaseColor = NewColor;
                }
                else
                {
                    BasicMaterialSettings.ShadowColor = NewColor;
                }

                StatusText->SetText(
                    FText::FromString(
                        TEXT("状態：カラー設定を変更しました。")
                    )
                );
            }
        )
    );

    PickerArgs.bIsModal = true;
    PickerArgs.bUseAlpha = false;

    // カラーピッカーを表示
    OpenColorPicker(PickerArgs);

    return FReply::Handled();    
}

void SToonShaderGeneratorWidget::HandleShadowThresholdChanged(float NewValue)
{
    // 0.0 ～ 1.0の範囲に制限
    BasicMaterialSettings.ShadowThreshold = FMath::Clamp(NewValue, 0.0f, 1.0f);
}

FReply SToonShaderGeneratorWidget::HandleApplyMaterialInstanceClicked(int32 SlotIndex)
{
    // Slotの検証
    if (!CurrentSlotInfos.IsValidIndex(SlotIndex))
    {
        StatusText->SetText(
            FText::FromString(
                TEXT("エラー：Material Slotが無効です。")
            )
        );
        
        return FReply::Handled();
    }
    
    FToonMaterialSlotInfo& SlotInfo = CurrentSlotInfos[SlotIndex];
    
    // 生成済みMIの確認
    if (SlotInfo.GeneratedMaterialInstancePath.IsNull())
    {
        StatusText->SetText(
            FText::FromString(
                TEXT("エラー：先にMaterial Instanceを生成してください。")
            )
        ); 
        
        return FReply::Handled();
    }
    
    // MaterialInstanceのロード
    UObject* LoadedMaterialObject = SlotInfo.GeneratedMaterialInstancePath.TryLoad();
    
    UMaterialInstanceConstant* MaterialInstance = Cast<UMaterialInstanceConstant>(LoadedMaterialObject);
    
    if (!IsValid(MaterialInstance))
    {
        StatusText->SetText(
            FText::FromString(
                TEXT("エラー：生成したMaterial Instanceを読み込めません。")
            )
        );
        return FReply::Handled();
    }
    
    // Skeletal Meshのロード
    UObject* LoadedMeshObject = SelectedSkeletalMeshPath.TryLoad();
    
    USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(LoadedMeshObject);
    
    if (!IsValid(SkeletalMesh))
    {
        StatusText->SetText(
            FText::FromString(
                TEXT("エラー：Skeletal Meshを読み込めません。")
            )
        );
        return FReply::Handled();
    }
    
    // 適用前の確認
    const FString ConfirmationMessage = FString::Printf(
        TEXT(
            "以下のMaterial Instanceを適用します。\n\n"
            "Skeletal Mesh：%s\n"
            "Material Slot：%s\n"
            "Material Instance：%s\n\n"
            "この変更は、このSkeletal Meshを使用する"
            "すべてのActorに影響します。\n\n"
            "実行しますか？"
            ),
            *SkeletalMesh->GetName(),
            *SlotInfo.SlotName.ToString(),
            *MaterialInstance->GetName()
    );
    
    const EAppReturnType::Type UserResponse = 
        FMessageDialog::Open(
            EAppMsgType::YesNo,
            FText::FromString(ConfirmationMessage)
        );
    
    if (UserResponse != EAppReturnType::Yes)
    {
        StatusText->SetText(
            FText::FromString(TEXT("状態：適用をキャンセルしました。"))
        );
        return FReply::Handled();
    }
    
    // Materialを適用
    FString ErrorMessage;
    
    const bool bSuccess = 
        FToonMeshMaterialAssignmentService::SetMaterialForSlot(
            SkeletalMesh,
            SlotIndex,
            MaterialInstance,
            ErrorMessage
        );
    
    if (!bSuccess)
    {
        StatusText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("適用失敗：%s"),
                    *ErrorMessage
                )
            )
        );
        return FReply::Handled();
    }
    
    //  UIの情報を更新
    
    SlotInfo.MaterialName = MaterialInstance->GetName();
    SlotInfo.bHasMaterial = true;
    
    // ウィジェット上のマテリアル名表示（Material：〇〇）を動的に更新
    if (TSharedPtr<STextBlock>* TextWidget = MaterialNameTextBlocks.Find(SlotIndex))
    {
        if (TextWidget->IsValid())
        {
            (*TextWidget)->SetText(
                FText::FromString(
                    FString::Printf(
                        TEXT("Material：%s"),
                        *SlotInfo.MaterialName
                    )
                )
            );
        }
    }
    
    StatusText->SetText(
        FText::FromString(
            FString::Printf(
                TEXT("適用成功：%s"),
                *MaterialInstance->GetName()
            )
        )
    );
    return FReply::Handled();
}

FReply SToonShaderGeneratorWidget::HandleRestoreOriginalMaterialClicked(int32 SlotIndex)
{
    // Slotの検証
    if (!CurrentSlotInfos.IsValidIndex(SlotIndex))
    {
        StatusText->SetText(
            FText::FromString(
                TEXT("エラー：Material Slotが無効です。")
            )
        );
        return FReply::Handled();
    }
    
    FToonMaterialSlotInfo& SlotInfo = CurrentSlotInfos[SlotIndex];
    
    //  バックアップの確認
    if (!SlotInfo.bHasOriginalMaterialBackup)
    {
        StatusText->SetText(
            FText::FromString(
                TEXT("エラー：元のMaterial情報がありません。")
            )
        );
        return FReply::Handled();
    }
    
    //  Skeletal Meshをロード
    UObject* LoadedMeshObject = SelectedSkeletalMeshPath.TryLoad();
    
    USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(LoadedMeshObject);
    
    if (!IsValid(SkeletalMesh))
    {
        StatusText->SetText(
            FText::FromString(
                TEXT("エラー：Skeletal Meshを読み込めません。")
            )
        );
        return FReply::Handled();
    }
    
    // 元のMaterialをロード
    UMaterialInterface* OriginalMaterial = nullptr;
    
    if (!SlotInfo.OriginalMaterialPath.IsNull())
    {
        UObject* LoadedOriginalObject =SlotInfo.OriginalMaterialPath.TryLoad();
        
        OriginalMaterial = Cast<UMaterialInterface>(LoadedOriginalObject);
        
        if (!IsValid(OriginalMaterial))
        {
            StatusText->SetText(
                FText::FromString(
                    TEXT("エラー：元のMaterialを読み込めません。")
                )
            );
            return FReply::Handled();
        }
    }
    
    // 復元確認
    const FString OriginalMaterialName = 
        IsValid(OriginalMaterial) 
        ? OriginalMaterial->GetName()
        : TEXT("未割り当て");
    
    const FString ConfirmationMessage = FString::Printf(
        TEXT(
            "Material Slot「%s」を元の状態に戻します。\n\n"
            "復元するMaterial：%s\n\n"
            "実行しますか？"
        ),
        *SlotInfo.SlotName.ToString(),
        *OriginalMaterialName
    );
    
    const EAppReturnType::Type UserResponse =
        FMessageDialog::Open(
            EAppMsgType::YesNo,
            FText::FromString(ConfirmationMessage)
        );
    
    if (UserResponse != EAppReturnType::Yes)
    {
        StatusText->SetText(
            FText::FromString(TEXT("状態：復元をキャンセルしました。"))
        );
        return FReply::Handled();
    }
    
    // 元のMaterialに戻す
    FString ErrorMessage;
    
    const bool bSuccess =
        FToonMeshMaterialAssignmentService::SetMaterialForSlot(
            SkeletalMesh,
            SlotIndex,
            OriginalMaterial,
            ErrorMessage
        );
    
    if (!bSuccess)
    {
        StatusText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("復元失敗：%s"),
                    *ErrorMessage
                )
            )
        );
        return FReply::Handled();
    }
    
    // UIを更新
    SlotInfo.bHasMaterial = IsValid(OriginalMaterial);
    
    SlotInfo.MaterialName = 
        IsValid(OriginalMaterial)
        ? OriginalMaterial->GetName()
        : TEXT("未割り当て");
    
    // ウィジェット上のマテリアル名表示（Material：〇〇）を動的に更新
    if (TSharedPtr<STextBlock>* TextWidget = MaterialNameTextBlocks.Find(SlotIndex))
    {
        if (TextWidget->IsValid())
        {
            (*TextWidget)->SetText(
                FText::FromString(
                    FString::Printf(
                        TEXT("Material：%s"),
                        *SlotInfo.MaterialName
                    )
                )
            );
        }
    }
    
    
    StatusText->SetText(
        FText::FromString(
            FString::Printf(
                TEXT("復元成功：%s"),
                *OriginalMaterialName
            )
        )
    );
    return FReply::Handled();
}

FReply SToonShaderGeneratorWidget::HandleValidateTexturesClicked()
{
    // Material Slotの確認
    if (CurrentSlotInfos.IsEmpty())
    {
        StatusText->SetText(
            FText::FromString(
                TEXT("エラー：先にMaterial Slot情報を取得してください。")
            )
        );
        return FReply::Handled();
    }
    
    // 検証結果を格納する配列
    TArray<FToonValidationIssue> ValidationIssues;
    
    // 全Material Slotを検証
    for (const FToonMaterialSlotInfo& SlotInfo : CurrentSlotInfos)
    {
        FToonTextureValidator::ValidateSlot(
            SlotInfo,
            ValidationIssues
        );    
    }
    
    // UIに結果を表示
    DisplayValidationResults(ValidationIssues);
    
    return FReply::Handled();
}

void SToonShaderGeneratorWidget::DisplayValidationResults(const TArray<FToonValidationIssue>& Issues)
{
    // 前回の検証結果を削除
    ValidationResultBox->ClearChildren();
    
    int32 ErrorCount = 0;
    int32 WarningCount = 0;
    
    // エラー・警告の数を集計
    for (const FToonValidationIssue& Issue : Issues)
    {
        if (Issue.Severity == EToonValidationSeverity::Error)
        {
            ++ErrorCount;
        }
        else
        {
            ++WarningCount;
        }
    }
    
    //  検証結果の概要
    const FString Summary = FString::Printf(
        TEXT("検証結果：エラー %d件 / 警告 %d件"),
        ErrorCount,
        WarningCount
    );
    
    ValidationSummaryText->SetText(FText::FromString(Summary));
    
    // 問題がなかった場合
    if (Issues.IsEmpty())
    {
        ValidationResultBox->AddSlot()
        .AutoHeight()
        [
            SNew(STextBlock)
            .Text(
                FText::FromString(
                    TEXT("問題は検出されませんでした。")
                )
            )
        ];

        StatusText->SetText(
            FText::FromString(
                TEXT("状態：テクスチャ検証に成功しました。")
            )
        );

        return;
    }
    
    // 個別の検証結果を表示
    for (const FToonValidationIssue& Issue : Issues)
    {
        const bool bIsError = 
            Issue.Severity == EToonValidationSeverity::Error;
        
        const FString SeverityText = 
            bIsError ? TEXT("[ERROR]") : TEXT("[WARNING]");
        
        const FString ResultText = FString::Printf(
            TEXT("%s Slot[%d] %s / %s\n%s"),
            *SeverityText,
            Issue.SlotIndex,
            *Issue.SlotName.ToString(),
            *Issue.InputName,
            *Issue.Message.ToString()
        );
        
        ValidationResultBox->AddSlot()
        .AutoHeight()
        .Padding(0.0f, 2.0f, 0.0f, 6.0f)
        [
           SNew(STextBlock)
           .Text(
               FText::FromString(ResultText)
           )
           .AutoWrapText(true)
       ];
    }
    
    // ステータス更新
    if (ErrorCount > 0)
    {
        StatusText->SetText(
            FText::FromString(
                TEXT("状態：テクスチャにエラーがあります。")
            )
        );
    }
    else
    {
        StatusText->SetText(
            FText::FromString(
                TEXT("状態：検証完了。警告があります。")
            )
        );
    }
}

