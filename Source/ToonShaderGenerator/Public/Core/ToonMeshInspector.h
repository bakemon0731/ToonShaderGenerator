
#pragma once

#include "CoreMinimal.h"
#include "Data/ToonMaterialSlotInfo.h"

class USkeletalMesh;

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
