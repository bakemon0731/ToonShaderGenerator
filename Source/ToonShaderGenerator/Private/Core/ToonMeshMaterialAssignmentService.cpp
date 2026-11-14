#include "Core/ToonMeshMaterialAssignmentService.h"

#include "Engine/SkeletalMesh.h"
#include "Engine/SkinnedAssetCommon.h"
#include "Materials/MaterialInterface.h"
#include "ScopedTransaction.h"

bool FToonMeshMaterialAssignmentService::SetMaterialForSlot(USkeletalMesh* SkeletalMesh, int32 SlotIndex,
	UMaterialInterface* Material, FString& OutErrorMessage)
{
	OutErrorMessage.Reset();
	
	// Skeletal Meshの検証
	if (!IsValid(SkeletalMesh))
	{
		OutErrorMessage = 
			TEXT("Skeletal Meshが無効です。");
		
		return false;
	}
	
	// Material Slotの検証
	const TArray<FSkeletalMaterial>& CurrentMaterials = SkeletalMesh->GetMaterials();
	
	if (!CurrentMaterials.IsValidIndex(SlotIndex))
	{
		OutErrorMessage =TEXT("指定されたMaterial Slotが存在しません。");
		return false;
	}
	
	// Material配列のコピー
	TArray<FSkeletalMaterial> UpdatedMaterials = CurrentMaterials;
	
	// 指定Slotだけ変更
	UpdatedMaterials[SlotIndex].MaterialInterface = Material;
	
	// Undo対応トランザクション
	const FScopedTransaction Transaction(
		FText::FromString(
			TEXT("Toon Shader: Material Slot Assignment")
		),
		true
	);
	
	// 変更前の状態をUndo用に記録
	SkeletalMesh->Modify();
	
	//  Materialを反映
	SkeletalMesh->SetMaterials(UpdatedMaterials);
	
	// Editorに変更を通知
	SkeletalMesh->PostEditChange();
	
	// 保存対象としてマーク
	SkeletalMesh->MarkPackageDirty();
	
	return true;
}
