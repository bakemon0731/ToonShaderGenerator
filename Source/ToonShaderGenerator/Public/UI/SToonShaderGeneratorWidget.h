#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class STextBlock;

class SToonShaderGeneratorWidget : public SCompoundWidget
{
public:
	
	SLATE_BEGIN_ARGS( SToonShaderGeneratorWidget ) {}
		
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);
	
private:
	
	FReply HandleTestButtonClicked();
	
	// UIの状態表示を更新するための参照
	TSharedPtr<STextBlock> StatusText;
};