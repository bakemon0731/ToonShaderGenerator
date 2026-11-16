#pragma once

#include "CoreMinimal.h"

class USkeletalMesh;

// ==========================================
// Face UV Raster Result
// ==========================================

struct FToonFaceUVRasterResult
{
	// 画像解像度
	int32 Resolution = 0;
	
	// 対象となった三角形数
	int32 TriangleCount = 0;
	
	// 描画されたピクセル数
	int32 RasterizedPixelCount = 0;

	// UVが重なっていたピクセル数
	int32 OverlappedPixelCount = 0;
	
	// 法線プレビューの保存先
	FString NormalPreviewPath;
	
	// UVカバレッジの保存先
	FString CoveragePreviewPath;
};

// ==========================================
// Face UV Rasterizer
// ==========================================

class FToonFaceUVRasterizer
{
public:
	static bool GenerateNormalPreview(
	   const USkeletalMesh* SkeletalMesh,
	   int32 MaterialSlotIndex,
	   int32 LODIndex,
	   int32 Resolution,
	   FToonFaceUVRasterResult& OutResult,
	   FString& OutErrorMessage
   );
};