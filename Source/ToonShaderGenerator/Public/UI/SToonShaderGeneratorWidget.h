#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "Widgets/SCompoundWidget.h"

struct FAssetData;
struct FToonMaterialSlotInfo;

class STextBlock;
class SVerticalBox;

class SToonShaderGeneratorWidget : public SCompoundWidget
{
public:
	
	SLATE_BEGIN_ARGS( SToonShaderGeneratorWidget ) {}
		
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

private:
	
	// Skeletal Meshの選択変更
	void HandleSkeletalMeshChanged(const FAssetData& AssetData);
	
	// 現在選択されているアセットのパスを取得
	FString GetSelectedSkeletalMeshPath() const;
	
	// Meshの情報取得ボタン
	FReply HandleInspectMeshClicked();
	
	// Material Slot一覧の表示更新
	void UpdateMaterialSlotList(const TArray<FToonMaterialSlotInfo>& SlotInfos);
	
private:
	
	// 選択されたSkeletal Meshの参照(アセットの参照を保持し、メモリには読み込まない)
	FSoftObjectPath SelectedSkeletalMeshPath;
	
	// 状態表示用UI
	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<STextBlock> SelectedMeshText;
	TSharedPtr<STextBlock> MaterialSlotCountText;
	
	// Material Slot一覧の表示領域
	TSharedPtr<SVerticalBox> MaterialSlotListBox;
	
	
	
};