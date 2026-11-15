#include "Core/ToonFaceMeshAnalyzer.h"

#include "Engine/SkeletalMesh.h"
#include "Engine/SkinnedAssetCommon.h"

#include "MeshDescription.h"
#include "SkeletalMeshAttributes.h"

// ==================================================
// Material Slot Analysis
// ==================================================

bool FToonFaceMeshAnalyzer::AnalyzeMaterialSlot(const USkeletalMesh* SkeletalMesh, int32 MaterialSlotIndex,
	int32 LODIndex, FToonFaceMeshAnalysisResult& OutResult, FString& OutErrorMessage)
{
	// ==========================================
	// 初期化
	// ==========================================
	OutResult = FToonFaceMeshAnalysisResult();
	OutErrorMessage.Reset();
	
	// ==========================================
	// Skeletal Mesh Validation
	// ==========================================
	if (!IsValid(SkeletalMesh))
	{
		OutErrorMessage = 
			TEXT("Skeletal Meshが無効です。");
		
		return false;
	}
	
	// ==========================================
	// Material Slot Validation
	// ==========================================
	
	const TArray<FSkeletalMaterial>& Materials = SkeletalMesh->GetMaterials();
	
	if (!Materials.IsValidIndex(MaterialSlotIndex))
	{
		OutErrorMessage = 
			TEXT("Material Slot Indexが無効です。");
		
		return false;
	}
	
	if (LODIndex < 0)
	{
		OutErrorMessage = 
			TEXT("LOD Indexが無効です。");
		
		return false;
	}
	
	OutResult.LODIndex = LODIndex;
	OutResult.MaterialSlotIndex = MaterialSlotIndex;
	
	const FSkeletalMaterial& SkeletalMaterial = Materials[MaterialSlotIndex];
	
	// ==========================================
	// Material Slot Name
	// ==========================================
	
	OutResult.MaterialSlotName = SkeletalMaterial.MaterialSlotName;
	
	if (OutResult.MaterialSlotName.IsNone())
	{
		OutResult.MaterialSlotName = SkeletalMaterial.ImportedMaterialSlotName;
	}
	
	TArray<FName> CandidateSlotNames;
	
	if (!SkeletalMaterial.MaterialSlotName.IsNone())
	{
		CandidateSlotNames.AddUnique(SkeletalMaterial.MaterialSlotName);
	}
	
	if (!SkeletalMaterial.ImportedMaterialSlotName.IsNone())
	{
		CandidateSlotNames.AddUnique(SkeletalMaterial.ImportedMaterialSlotName);
	}
	
	if (CandidateSlotNames.IsEmpty())
	{
		OutErrorMessage =
			TEXT("Material Slot名を取得できません。");
		
		return false;
	}
	
	// ==========================================
	// MeshDescription取得
	// ==========================================
	
	const FMeshDescription* MeshDescription = SkeletalMesh->GetMeshDescription(LODIndex);
	
	if (!MeshDescription)
	{
		OutErrorMessage = FString::Printf(
			TEXT(
				"LOD %d のMeshDescriptionを取得できません。\n"
				"インポートデータが存在しないか、"
				"自動生成LODの可能性があります。"
			),
			LODIndex
		);
		return false;
	}
	
	// ==========================================
	// UV Channel確認
	// ==========================================
	
	OutResult.UVChannelCount = MeshDescription->GetNumUVElementChannels();
	
	if (OutResult.UVChannelCount <= 0)
	{
		OutErrorMessage =
			TEXT("UV Channelが存在しません。");
		
		return false;
	}
	
	// ==========================================
	// Mesh Attributes
	// ==========================================
	
	FSkeletalMeshAttributes MeshAttributes(
		*const_cast<FMeshDescription*>(MeshDescription));
	
	const auto PolygonGroupMaterialSlotNames = 
		MeshAttributes.GetPolygonGroupMaterialSlotNames();
	
	const auto VertexInstanceUVs = 
		MeshAttributes.GetVertexInstanceUVs();
	
	// ==========================================
	// UV Bounds初期化
	// ==========================================
	
	OutResult.UVMin = FVector2f(
		TNumericLimits<float>::Max(),
		TNumericLimits<float>::Max()
	);
	
	OutResult.UVMax = FVector2f(
	   TNumericLimits<float>::Lowest(),
	   TNumericLimits<float>::Lowest()
	);   
	
	// ==========================================
	// Polygon Group Analysis
	// ==========================================
	
	TArray<FName> AvailablePolygonGroupNames;
	
	bool bFoundMatchingPolygonGroup = false;
	
	for (const FPolygonGroupID PolygonGroupID : 
		MeshDescription->PolygonGroups().GetElementIDs())
	{
		const FName GroupSlotName = 
			PolygonGroupMaterialSlotNames[PolygonGroupID];
		
		AvailablePolygonGroupNames.AddUnique(
			GroupSlotName
		);
		
		// --------------------------------------
		// Material Slot Name Matching
		// --------------------------------------
		
		if (!CandidateSlotNames.Contains(GroupSlotName))
		{
			continue;
		}
		
		bFoundMatchingPolygonGroup = true;
		
		++OutResult.MatchingPolygonGroupCount;
		
		// --------------------------------------
		// Triangle取得
		// --------------------------------------
		
		const auto Triangles =
			MeshDescription->GetPolygonGroupTriangles(
				PolygonGroupID
			);
		
		for (const FTriangleID TriangleID : Triangles)
		{
			++OutResult.TriangleCount; 
			
			const auto TriangleVertexInstances =
			   MeshDescription->GetTriangleVertexInstances(
				   TriangleID
			   );
			
			for (const FVertexInstanceID VertexInstanceID :
				TriangleVertexInstances)
			{
				// UV Channel 0
				const FVector2f UV = VertexInstanceUVs.Get(VertexInstanceID,0);
				
				if (!FMath::IsFinite(UV.X) || !FMath::IsFinite(UV.Y))
				{
					continue;
				}
				
				++OutResult.UVSampleCount;

				OutResult.UVMin.X =
					FMath::Min(OutResult.UVMin.X, UV.X);

				OutResult.UVMin.Y =
					FMath::Min(OutResult.UVMin.Y, UV.Y);

				OutResult.UVMax.X =
					FMath::Max(OutResult.UVMax.X, UV.X);

				OutResult.UVMax.Y =
					FMath::Max(OutResult.UVMax.Y, UV.Y);

				OutResult.bHasUVBounds = true;
			}
		}
	}
	
	// ==========================================
	// Matching Result Validation
	// ==========================================

	if (!bFoundMatchingPolygonGroup)
	{
		TArray<FString> AvailableNames;

		for (const FName GroupName : AvailablePolygonGroupNames)
		{
			AvailableNames.Add(
				GroupName.IsNone()
					? TEXT("(None)")
					: GroupName.ToString()
			);
		}

		OutErrorMessage = FString::Printf(
			TEXT(
				"Material Slotに対応するPolygon Groupがありません。\n"
				"対象Slot：%s\n"
				"検出したPolygon Group：%s"
			),
			*OutResult.MaterialSlotName.ToString(),
			*FString::Join(AvailableNames, TEXT(", "))
		);

		return false;
	}

	if (OutResult.TriangleCount <= 0)
	{
		OutErrorMessage =
			TEXT("対象Material Slotに三角形がありません。");

		return false;
	}

	if (!OutResult.bHasUVBounds)
	{
		OutErrorMessage =
			TEXT("対象Material SlotのUVを取得できません。");

		return false;
	}


	// ==========================================
	// UV Range Validation
	// ==========================================

	constexpr float UVTolerance = 0.001f;

	OutResult.bUVsWithinZeroToOne =
		OutResult.UVMin.X >= -UVTolerance
		&& OutResult.UVMin.Y >= -UVTolerance
		&& OutResult.UVMax.X <= 1.0f + UVTolerance
		&& OutResult.UVMax.Y <= 1.0f + UVTolerance;


	return true;
}
