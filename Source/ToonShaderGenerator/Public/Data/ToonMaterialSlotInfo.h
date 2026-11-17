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
	// 自動生成したFace Normal Map
	FSoftObjectPath FaceNormalDirectionTexturePath;

	// FaceSDFがプラグイン生成のRG形式(R=右光/G=左光)か
	// false = 手描きなどの1チャンネルSDF(UVのU反転で左右を切り替える)
	bool bFaceSDFIsGenerated = false;

	// 顔のローカル空間での前方向・右方向(SDF生成時に自動検出した値)
	FVector3f FaceForwardAxis = FVector3f(0.0f, 1.0f, 0.0f);
	FVector3f FaceRightAxis = FVector3f(-1.0f, 0.0f, 0.0f);
	
	// Hair用
	FSoftObjectPath HairFlowMapTexturePath;
	
	// Skin用
	FSoftObjectPath SkinBaseColorTexturePath;
	
	// Clothing用
	FSoftObjectPath ClothingBaseColorTexturePath;
	FSoftObjectPath ClothingNormalTexturePath;
	FSoftObjectPath ClothingORMTexturePath;
	
	// 適用前の元のMaterial
	FSoftObjectPath OriginalMaterialPath;
	
	// 元のMaterial情報を取得済みか
	bool bHasOriginalMaterialBackup = false;
	
	// 生成したMaterial Instance
	FSoftObjectPath GeneratedMaterialInstancePath;
	
};
