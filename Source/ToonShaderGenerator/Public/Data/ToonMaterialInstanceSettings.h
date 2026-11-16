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
	FSoftObjectPath FaceSDFTexturePath;
	
	FSoftObjectPath FaceNormalDirectionTexturePath;
	
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
