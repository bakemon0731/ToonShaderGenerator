#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "Widgets/SCompoundWidget.h"

struct FAssetData;

class STextBlock;

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
	
private:
	
	// 状態表示用UI
	TSharedPtr<STextBlock> StatusText;
	
	// 選択されたSkeletal Meshの参照(アセットの参照を保持し、メモリには読み込まない)
	FSoftObjectPath SelectedSkeletalMeshPath;
	
	// 選択中のアセット名を表示するUI
	TSharedPtr<STextBlock> SelectedMeshText;
	
};