#pragma once

#include "CoreMinimal.h"

#include "Data/ToonBasicMaterialSettings.h"
#include "Data/ToonMaterialSlotInfo.h"
#include "Data/ToonHairSettings.h"

// Material Instance生成時の設定
struct FToonMaterialInstanceSettings
{
	// Material Slotの用途
	EToonMaterialType MaterialType = EToonMaterialType::Other;
	
	// 基本シェーダー設定
	FToonBasicMaterialSettings Basic;
	
	// ==========================================
	// Face Shader
	// ==========================================
	// 未指定ならマスターマテリアル固定の標準SDFを使う(上書きしない)
	FSoftObjectPath FaceSDFTexturePath;

	// 未指定ならマスターマテリアルの既定テクスチャのまま
	FSoftObjectPath FaceBaseColorTexturePath;


	// FaceSDFをサンプルするUVの調整
	// サンプルV = (V - 0.5) * FaceSDFVScale + 0.5 + FaceSDFVOffset
	float FaceSDFVOffset = 0.0f;
	float FaceSDFVScale = 1.0f;

	// 顔のローカル空間での前方向・右方向
	FVector3f FaceForwardAxis = FVector3f(0.0f, 1.0f, 0.0f);
	FVector3f FaceRightAxis = FVector3f(-1.0f, 0.0f, 0.0f);
	
	float FaceBoundarySoftness = 0.02f;
	
	// ==========================================
	// Hair Shader
	// ==========================================
	FSoftObjectPath HairBaseColorTexturePath;

	FSoftObjectPath HairFlowMapTexturePath;
	
	// Hairのトゥーン設定(ハイライト・毛束方向の陰影)
	FToonHairSettings Hair;
	
	// Flow Mapの有効・無効
	bool bUseHairFlowMap = false;
	
	// ==========================================
	// Skin Shader
	// ==========================================
	
	FSoftObjectPath SkinBaseColorTexturePath;
	
	// ==========================================
	// Clothing Shader
	// ==========================================
	
	FSoftObjectPath ClothingBaseColorTexturePath;

	FSoftObjectPath ClothingNormalTexturePath;

	FSoftObjectPath ClothingORMTexturePath;
};
