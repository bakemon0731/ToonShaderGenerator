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

	// FaceSDFの保存先(R=右から光が来る場合 / G=左から光が来る場合)
	FString SDFPreviewPath;
};

// ==========================================
// Face SDF Settings
// ==========================================

struct FToonFaceSDFSettings
{
	// SDFも同時に生成する
	bool bGenerate = true;

	// 顔の前方向(スケルタルメッシュのローカル空間)
	// 例: Y軸(緑)が前なら(0,1,0)
	FVector3f ForwardAxis = FVector3f(0.0f, 1.0f, 0.0f);

	// キャラクターから見た右方向(ローカル空間)
	// 前=+Y、上=+Zの場合、右は-X
	FVector3f RightAxis = FVector3f(-1.0f, 0.0f, 0.0f);

	// 明暗の境界となる Dot(法線, ライト) の値
	// 0.0で「光が面に垂直より手前なら明るい」
	float ShadowThreshold = 0.0f;
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
	   FString& OutErrorMessage,
	   // 0.0 = 元の頂点法線のまま / 1.0 = 完全に球状(中心から外向き)の法線
	   // 顔の影境界を滑らかにするため、0.7～1.0程度を推奨
	   float NormalSmoothingStrength = 0.85f,
	   const FToonFaceSDFSettings& SDFSettings = FToonFaceSDFSettings()
   );
};