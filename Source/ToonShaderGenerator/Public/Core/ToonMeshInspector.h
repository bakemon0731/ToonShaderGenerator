
#pragma once

#include "CoreMinimal.h"

class USkeletalMesh;

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
};


class FToonMeshInspector
{
public:

	// Skeletal MeshのMaterial Slot情報を取得する
	static bool GetMaterialSlotInfos(
		const USkeletalMesh* SkeletalMesh,
		TArray<FToonMaterialSlotInfo>& OutSlotInfos,
		FString& OutErrorMessage
	);
};
