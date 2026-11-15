#pragma once

#include "CoreMinimal.h"

class USkeletalMesh;

// ==========================================
// Face Mesh Analysis Result
// ==========================================
struct FToonFaceMeshAnalysisResult
{
	// 解析対象LOD
	int32 LODIndex = INDEX_NONE;
	
	// Material Slot Index
	int32 MaterialSlotIndex = INDEX_NONE;
	
	// Material Slot Name
	FName MaterialSlotName = NAME_None;
	
	// UV Channel数
	int32 UVChannelCount = 0;
	
	// Material Slotに対応するPolygon Group数
	int32 MatchingPolygonGroupCount = 0;
	
	// 三角形数
	int32 TriangleCount = 0;
	
	// 取得したUVサンプル数
	int32 UVSampleCount = 0;
	
	// UV最小値
	FVector2f UVMin = FVector2f(0.0f, 0.0f);
	
	// UV最大値
	FVector2f UVMax = FVector2f(0.0f, 0.0f);
	
	// UV情報を取得できたか
	bool bHasUVBounds = false;
	
	// UVが0～1の範囲に収まっているか
	bool bUVsWithinZeroToOne = false;
};

// ==========================================
// Face Mesh Analyzer
// ==========================================

class FToonFaceMeshAnalyzer
{
public:
	
	static bool AnalyzeMaterialSlot(
	   const USkeletalMesh* SkeletalMesh,
	   int32 MaterialSlotIndex,
	   int32 LODIndex,
	   FToonFaceMeshAnalysisResult& OutResult,
	   FString& OutErrorMessage
   );
};