
#pragma once

#include "CoreMinimal.h"
#include "Data/ToonMaterialSlotInfo.h"
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
	
	// 選択されたMaster Materialの参照
	FSoftObjectPath SelectedMasterMaterialPath;

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
	
	// Master Materialの選択変更
	void HandleMasterMaterialChanged(const FAssetData& AssetData);

	// 選択中のMaster Materialのパス
	FString GetSelectedMasterMaterialPath() const;

	// Material Instance生成ボタン
	FReply HandleGenerateMaterialInstanceClicked(int32 SlotIndex);
};
