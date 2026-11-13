
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
};
