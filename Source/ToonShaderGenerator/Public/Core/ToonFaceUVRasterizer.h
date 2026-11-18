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

	// SDF生成に使った顔の前方向・右方向(ローカル空間)
	FVector3f SDFForwardAxis = FVector3f(0.0f, 1.0f, 0.0f);
	FVector3f SDFRightAxis = FVector3f(-1.0f, 0.0f, 0.0f);

	// SDFに出っ張り補正(鼻先・頬・あご)を適用したか
	bool bSDFFeatureApplied = false;
};

// ==========================================
// Face SDF Settings
// ==========================================

struct FToonFaceSDFSettings
{
	// SDFも同時に生成する
	bool bGenerate = true;

	// 顔の前方向を自動検出する
	// 顔スロットの頂点法線の平均(の水平成分)を前方向とするため、
	// 前が+Yでも-Xでも、反対向きのキャラクターでも対応できる
	bool bAutoDetectAxes = true;

	// 頭の上方向(ローカル空間)。右方向 = Cross(Up, Forward)
	FVector3f UpAxis = FVector3f(0.0f, 0.0f, 1.0f);

	// 以下のForward/Rightは、自動検出できなかった場合(または自動検出OFF時)に使う

	// 顔の前方向(スケルタルメッシュのローカル空間)
	// 例: Y軸(緑)が前なら(0,1,0)
	FVector3f ForwardAxis = FVector3f(0.0f, 1.0f, 0.0f);

	// キャラクターから見た右方向(ローカル空間)
	// 前=+Y、上=+Zの場合、右は-X
	FVector3f RightAxis = FVector3f(-1.0f, 0.0f, 0.0f);

	// 明暗の境界となる Dot(法線, ライト) の値
	// 0.0で「光が面に垂直より手前なら明るい」
	float ShadowThreshold = 0.0f;

	// 出っ張り補正の強さ(0.0〜0.3程度)。SDF値(0〜1 = 0〜180度)に加算する。
	// 0.16 = 約29度。大きいほど、出っ張りが影側でも光を長く受ける。
	// へこみ(眼窩など)は逆に早く影になる。0.0で無効。
	//
	// 鼻先・あご・眉など「細かい出っ張り」
	float NoseTipGain = 0.16f;

	// 頬・頬骨など「顔の側面にある広い出っ張り」(顔の中心線付近の鼻先・あごには効かない)
	float CheekGain = 0.3f;
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