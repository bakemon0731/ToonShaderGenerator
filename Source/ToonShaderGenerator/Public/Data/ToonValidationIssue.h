#pragma once

#include "CoreMinimal.h"

// 検証結果の重要度
enum class EToonValidationSeverity : uint8
{
	Warning,
	Error
};

// 1件の検証結果
struct FToonValidationIssue
{
	// 対象Material Slot
	int32 SlotIndex = INDEX_NONE;
	
	// Material Slot名
	FName SlotName = NAME_None;
	
	// 対象テクスチャ名
	FString InputName;
	
	// エラーか警告か
	EToonValidationSeverity Severity = EToonValidationSeverity::Warning;
	
	// 検証結果の説明
	FText Message;
};
