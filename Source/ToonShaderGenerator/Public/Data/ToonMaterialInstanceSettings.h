#pragma once

#include "CoreMinimal.h"

#include "Data/ToonBasicMaterialSettings.h"
#include "Data/ToonMaterialSlotInfo.h"

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


	// 顔のローカル空間での前方向・右方向
	FVector3f FaceForwardAxis = FVector3f(0.0f, 1.0f, 0.0f);
	FVector3f FaceRightAxis = FVector3f(-1.0f, 0.0f, 0.0f);
	
	float FaceBoundarySoftness = 0.02f;
	
	// ==========================================
	// Hair Shader
	// ==========================================
	FSoftObjectPath HairFlowMapTexturePath;
	
	float HairAnisotropy = 0.8f;
	
	float HairRoughness = 0.35f;
	
	float HairSpecular = 0.5f;
	
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
