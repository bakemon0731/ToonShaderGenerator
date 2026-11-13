#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"

// Material Slotの用途
enum class EToonMaterialType : uint8
{
	Face,
	Hair,
	Skin,
	Clothing,
	Other
};

// テクスチャ入力の種類
enum class EToonTextureInputType : uint8
{
	FaceSDF,
	HairFlowMap,
	SkinBaseColor,
	ClothingBaseColor,
	ClothingNormal,
	ClothingORM
};

// Material Slotの情報を保持する
struct FToonMaterialSlotInfo
{
	// Slotの配列番号
	int32 SlotIndex = INDEX_NONE;

	// Material Slot名
	FName SlotName = NAME_None;

	// 現在割り当てられているMaterial名
	FString MaterialName;

	// Materialが割り当てられているか
	bool bHasMaterial = false;
	
	// ツール内で指定する用途
	EToonMaterialType MaterialType = EToonMaterialType::Other;
	
	// Face用
	FSoftObjectPath FaceSDFTexturePath;
	
	// Hair用
	FSoftObjectPath HairFlowMapTexturePath;
	
	// Skin用
	FSoftObjectPath SkinBaseColorTexturePath;
	
	// Clothing用
	FSoftObjectPath ClothingBaseColorTexturePath;
	FSoftObjectPath ClothingNormalTexturePath;
	FSoftObjectPath ClothingORMTexturePath;
};
