#include "Core/ToonMeshInspector.h"

#include "Engine/SkeletalMesh.h"
#include "Engine/SkinnedAssetCommon.h"
#include "Materials/MaterialInterface.h"

bool FToonMeshInspector::GetMaterialSlotInfos(const USkeletalMesh* SkeletalMesh,
	TArray<FToonMaterialSlotInfo>& OutSlotInfo, FString& OutErrorMessage)
{
	// 前回の結果をクリア
	OutSlotInfo.Reset();
	OutErrorMessage.Empty();
	
	// Skeletal Meshの有効性を確認
	if (!IsValid(SkeletalMesh))
	{
		OutErrorMessage = TEXT("Skeletal Meshが無効です。");
		return false;
	}
	
	// Skeletal MeshからMaterial Slotの配列を取得
	const TArray<FSkeletalMaterial>& Materials = SkeletalMesh->GetMaterials();
	
	// Material Slotを1つずつ確認
	for (int32 Index = 0; Index < Materials.Num(); ++Index)
	{
		// 現在確認しているSlotの情報を取得
		const FSkeletalMaterial& MaterialInfo = Materials[Index];
		
		// 情報を格納する構造体
		FToonMaterialSlotInfo SlotInfo;
		
		// 配列番号
		SlotInfo.SlotName = MaterialInfo.MaterialSlotName;
		
		// Slot名が存在しない場合はImported名を使用
		if (SlotInfo.SlotName.IsNone())
		{
			SlotInfo.SlotName = MaterialInfo.ImportedMaterialSlotName;
		}
		
		// Material Interfaceを取得
		UMaterialInterface* MaterialInterface = MaterialInfo.MaterialInterface.Get();
		
		// Materialが設定されているか確認
		SlotInfo.bHasMaterial = IsValid(MaterialInterface);
		
		if (SlotInfo.bHasMaterial)
		{
			SlotInfo.MaterialName = MaterialInterface->GetName();
		}
		else
		{
			SlotInfo.MaterialName = TEXT("未割り当て");
		}
		
		// 取得した情報を配列に追加
		OutSlotInfo.Add(MoveTemp(SlotInfo));
	}
	return true;
}
