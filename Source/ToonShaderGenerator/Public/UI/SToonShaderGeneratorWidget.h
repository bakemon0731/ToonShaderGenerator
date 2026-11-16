
#pragma once

#include "CoreMinimal.h"
#include "Data/ToonMaterialSlotInfo.h"
#include "Data/ToonBasicMaterialSettings.h"
#include "Data/ToonValidationIssue.h"
#include "UObject/SoftObjectPath.h"
#include "Widgets/SCompoundWidget.h"

struct FAssetData;

class STextBlock;
class SVerticalBox;

class SToonShaderGeneratorWidget : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SToonShaderGeneratorWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
	// Material Slotごとのテクスチャ入力UI
	TMap<int32, TSharedPtr<SVerticalBox>> MaterialInputBoxes;
	
	// 基本Toon Shaderの設定
	FToonBasicMaterialSettings BasicMaterialSettings;
	
	// Hair Shader Settings
	float HairRoughness = 0.35f;
	float HairSpecular = 0.5f;
	float HairAnisotropy = 0.8f;
	bool bUseHairFlowMap = false;
	
	// Material名表示WidgetをSlotごとに保持
	TMap<int32, TSharedPtr<STextBlock>>  MaterialNameTextBlocks;
	
	// 検証結果の概要
	TSharedPtr<STextBlock> ValidationSummaryText;

	// 検証結果の一覧
	TSharedPtr<SVerticalBox> ValidationResultBox;

private:

	// Skeletal Meshの選択変更
	void HandleSkeletalMeshChanged(const FAssetData& AssetData);

	// 選択されたMeshのパス取得
	FString GetSelectedSkeletalMeshPath() const;

	// Mesh情報取得
	FReply HandleInspectMeshClicked();

	// Material Slot一覧更新
	void UpdateMaterialSlotList(
		const TArray<FToonMaterialSlotInfo>& SlotInfos
	);

	// 用途変更時
	void HandleMaterialTypeChanged(
		TSharedPtr<EToonMaterialType> SelectedType,
		int32 SlotIndex
	);

	// 現在の用途を表示するテキスト取得
	FText GetMaterialTypeText(int32 SlotIndex) const;

	// 用途に対応する選択肢を取得
	TSharedPtr<EToonMaterialType> FindMaterialTypeOption(
		EToonMaterialType MaterialType
	) const;
	
	// Master Materialの表示
	TSharedPtr<STextBlock> MasterMaterialText;

private:

	// 選択中のSkeletal Mesh
	FSoftObjectPath SelectedSkeletalMeshPath;

	// 取得済みのMaterial Slot情報
	TArray<FToonMaterialSlotInfo> CurrentSlotInfos;

	// 用途選択肢
	TArray<TSharedPtr<EToonMaterialType>> MaterialTypeOptions;

	// UI
	TSharedPtr<STextBlock> SelectedMeshText;
	TSharedPtr<STextBlock> MaterialSlotCountText;
	TSharedPtr<STextBlock> StatusText;

	TSharedPtr<SVerticalBox> MaterialSlotListBox;
	
private:
	
	TSharedRef<SWidget> CreateTextureInputWidget(int32 SlotIndex,EToonTextureInputType InputType,const FText& Label);

	// 選択中のテクスチャパスを取得
	FString GetTexturePath(int32 SlotIndex,EToonTextureInputType InputType) const;

	// テクスチャ変更時の処理
	void HandleTextureChanged(const FAssetData& AssetData,int32 SlotIndex,EToonTextureInputType InputType);

	// 用途に応じて入力欄を更新
	void RefreshMaterialInputWidgets(int32 SlotIndex);
	
private:

	// Material Instance生成ボタン
	FReply HandleGenerateMaterialInstanceClicked(int32 SlotIndex);
	
private:
	
	// カラーピッカーを開く
	FReply HandleOpenColorPicker(bool bEditBaseColor);
	
	// Shadow Threshold変更
	void HandleShadowThresholdChanged(float NewValue);
	
private:
	
	// 生成したMaterial Instanceを適用
	FReply HandleApplyMaterialInstanceClicked(int32 SlotIndex);
	
	// 元のMaterialに戻す
	FReply HandleRestoreOriginalMaterialClicked(int32 SlotIndex);
	
private:
	
	// テクスチャ検証ボタン
	FReply HandleValidateTexturesClicked();

	// 検証結果をUIに表示
	void DisplayValidationResults(const TArray<FToonValidationIssue>& Issues);
	
private:
	
	void HandleHairRoughnessChanged(float NewValue);
	void HandleHairSpecularChanged(float NewValue);
	void HandleHairAnisotropyChanged(float NewValue);
	
private:
	// Face Mesh Analysis
	FReply HandleAnalyzeFaceSlotsClicked();
	
	FReply HandleGenerateFaceNormalPreviewClicked();
	
	FReply HandleImportFaceNormalPreviewClicked();
};
