#pragma once

#include "CoreMinimal.h"

// Hair Toon Shaderの設定値
// BaseColor / ShadowColor / ShadowThreshold は共通の基本設定を使う
struct FToonHairSettings
{
	// 影の境界のぼかし幅
	float ShadowSoftness = 0.1f;

	// 毛束に沿った法線への寄せ具合(0=元の法線 / 1=毛束方向の成分を完全に除去)
	float FlowNormalBlend = 0.8f;

	// Kajiya-Kay拡散(ライトと毛束の角度)の混ぜ具合
	float StrandDiffuseBlend = 0.3f;
};
