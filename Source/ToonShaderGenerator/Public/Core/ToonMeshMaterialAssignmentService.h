#pragma once

#include "CoreMinimal.h"

class USkeletalMesh;
class UMaterialInterface;

class FToonMeshMaterialAssignmentService
{
public:
	
	// 指定したMaterial SlotにMaterialを設定する
	// nullptrを渡すとMaterialを解除できる
	static bool SetMaterialForSlot(
		USkeletalMesh* SkeletalMesh,
		int32 SlotIndex,
		UMaterialInterface* Material,
		FString& OutErrorMessage
	);	
};
