#pragma once

#include "Data/ToonMaterialSlotInfo.h"
#include "Data/ToonValidationIssue.h"

class FToonTextureValidator
{
public:
	
	// Material Slotのテクスチャを検証
	static void ValidateSlot(
		const FToonMaterialSlotInfo& SlotInfo,
		TArray<FToonValidationIssue>& OutIssues
	);	
};