#pragma once

#include "CoreMinimal.h"

// 基本Toon Shaderの設定値
struct FToonBasicMaterialSettings
{
	// 明るい部分の色
	FLinearColor BaseColor = FLinearColor::White;
	
	// 影の色
	FLinearColor ShadowColor = FLinearColor(0.06f, 0.08f, 0.16f, 1.0f);
	
	// 影の境界
	float ShadowThreshold = 0.2f;
};