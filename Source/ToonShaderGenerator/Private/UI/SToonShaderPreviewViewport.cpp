#include "UI/SToonShaderPreviewViewport.h"

#include "AdvancedPreviewScene.h"
#include "EditorViewportClient.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Engine/SkinnedAssetCommon.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Texture.h"


// ==================================================
// コンストラクタ
// ==================================================

void SToonShaderPreviewViewport::Construct(
	const FArguments& InArgs)
{
	// プレビューシーンの設定
	FPreviewScene::ConstructionValues SceneSettings;

	// デフォルトのライティングを有効化
	SceneSettings.SetCreateDefaultLighting(true);

	// プレビュー専用空間を生成
	PreviewScene =
		MakeShared<FAdvancedPreviewScene>(
			SceneSettings,
			0.0f
		);

	// 親クラスのViewportを初期化
	SEditorViewport::Construct(
		SEditorViewport::FArguments()
	);
}

// ==================================================
// Viewport Client生成
// ==================================================

TSharedRef<FEditorViewportClient>
SToonShaderPreviewViewport::MakeEditorViewportClient()
{
	// Viewport Clientを生成
	TSharedRef<FEditorViewportClient> ViewportClient =
		MakeShared<FEditorViewportClient>(
			nullptr,
			PreviewScene.Get(),
			SharedThis(this)
		);

	// カメラの位置
	ViewportClient->SetViewLocation(
		FVector(-400.0f, 0.0f, 120.0f)
	);

	// カメラの向き
	ViewportClient->SetViewRotation(
		FRotator(-10.0f, 0.0f, 0.0f)
	);

	// 描画モード
	ViewportClient->SetViewMode(VMI_Lit);

	return ViewportClient;
}

void SToonShaderPreviewViewport::SetPreviewSkeletalMesh(USkeletalMesh* NewMesh)
{
	// Preview Sceneが存在しない場合は終了
	if (!PreviewScene.IsValid())
	{
		return;
	}
	
	// 既存のプレビューコンポーネントを削除
	if (PreviewMeshComponent)
	{
		PreviewScene->RemoveComponent(PreviewMeshComponent);
		
		PreviewMeshComponent = nullptr;
	}
	
	// Meshが指定されていない場合
	if (!IsValid(NewMesh))
	{
		Invalidate();
		return;
	}
	
	// プレビュー用コンポーネントを生成
	PreviewMeshComponent = NewObject<USkeletalMeshComponent>(
		GetTransientPackage(),
		NAME_None,
		RF_Transient
	);
	
	if (!IsValid(PreviewMeshComponent))
	{
		return;
	}
	
	// 表示するSkeletal Meshを設定
	PreviewMeshComponent->SetSkeletalMeshAsset(NewMesh);
	
	// Preview Sceneに追加
	PreviewScene->AddComponent(
		PreviewMeshComponent,
		FTransform::Identity,
		true
	);
	
	// Meshに合わせてカメラを初期化
	ResetCamera();
}

// ==================================================
// カメラを初期視点に戻す
// ==================================================
void SToonShaderPreviewViewport::ResetCamera()
{
	// プレビュー中のMeshが存在しない場合
	if (!IsValid(PreviewMeshComponent))
	{
		return;
	}
	
	// Viewport Clientを取得
	const TSharedPtr<FEditorViewportClient> ViewportClient = GetViewportClient();
	
	if (!ViewportClient.IsValid())
	{
		return;
	}
	
	// Meshの大きさと中心位置を取得
	const FBoxSphereBounds MeshBounds = PreviewMeshComponent->Bounds;
	
	// Meshのサイズからカメラ距離を計算
	const float CameraDistance = FMath::Max(MeshBounds.SphereRadius * 2.5f, 300.0f);
	
	// 初期角度に戻す
	ViewportClient->SetViewRotation(FRotator(-10.0f, 0.0f, 0.0f));
	
	// Meshの中心にカメラを合わせる
	ViewportClient->SetViewLocationForOrbiting(MeshBounds.Origin, CameraDistance);
	
	// 再描画
	ViewportClient->Invalidate(false, false);
	Invalidate();
}

// ==================================================
// プレビュー専用Materialを設定
// ==================================================
bool SToonShaderPreviewViewport::SetPreviewMaterialForSlot(int32 SlotIndex, UMaterialInterface* SourceMaterial)
{
	// Preview Meshの確認
	if (!IsValid(PreviewMeshComponent))
	{
		return false;
	}
	
	// Materialの確認
	if (!IsValid(SourceMaterial))
	{
		return false;
	}
	
	// Slotの範囲確認
	if (SlotIndex < 0 || SlotIndex >= PreviewMeshComponent->GetNumMaterials())
	{
		return false;
	}
	
	// Preview専用MIDを生成してSlotに設定
	UMaterialInstanceDynamic* PreviewMID = PreviewMeshComponent->CreateDynamicMaterialInstance(
		SlotIndex,
		SourceMaterial,
		NAME_None
	);
	
	if (!IsValid(PreviewMID))
	{
		return false;
	}
	
	// Viewportの更新
	const TSharedPtr<FEditorViewportClient> ViewportClient = GetViewportClient();
	
	if (ViewportClient.IsValid())
	{
		ViewportClient->Invalidate(false, false);
	}
	
	Invalidate();
	
	return true;
}

// ==================================================
// プレビュー用MIDの取得
// ==================================================
UMaterialInstanceDynamic* SToonShaderPreviewViewport::GetPreviewMaterialInstanceDynamic(int32 SlotIndex) const
{
	if (!IsValid(PreviewMeshComponent))
	{
		return nullptr;
	}

	if (SlotIndex < 0 || SlotIndex >= PreviewMeshComponent->GetNumMaterials())
	{
		return nullptr;
	}

	return Cast<UMaterialInstanceDynamic>(
		PreviewMeshComponent->GetMaterial(SlotIndex)
	);
}

// ==================================================
// プレビューのMaterialを元の状態に戻す
// ==================================================
bool SToonShaderPreviewViewport::RestorePreviewMaterialForSlot(int32 SlotIndex)
{
	if (!IsValid(PreviewMeshComponent))
	{
		return false;
	}
	
	USkeletalMesh* SourceMesh = PreviewMeshComponent->GetSkeletalMeshAsset();
	
	if (!IsValid(SourceMesh))
	{
		return false;
	}
	
	const TArray<FSkeletalMaterial>& Materials = SourceMesh->GetMaterials();
	
	if (!Materials.IsValidIndex(SlotIndex))
	{
		return false;
	}
	
	// 元のSkeletal Meshが持つMaterialを取得
	UMaterialInterface* OriginalMaterial = Materials[SlotIndex].MaterialInterface.Get();
	
	// Preview Componentだけ元に戻す
	PreviewMeshComponent->SetMaterial(SlotIndex, OriginalMaterial);
	
	// 再描画
	const TSharedPtr<FEditorViewportClient> ViewportClient = GetViewportClient();
	
	if (ViewportClient.IsValid())
	{
		ViewportClient->Invalidate(false, false);
	}
	Invalidate();
	return true;
}

// ==================================================
// プレビューMaterialのパラメータ更新
// ==================================================
bool SToonShaderPreviewViewport::UpdatePreviewMaterialParameters(int32 SlotIndex, FName BaseColorParameterName,
	const FLinearColor& BaseColor, FName ShadowColorParameterName, const FLinearColor& ShadowColor,
	FName ShadowThresholdParameterName, float ShadowThreshold)
{
	// Preview Meshの確認
	if (!IsValid(PreviewMeshComponent))
	{
		return false;
	}
	
	if (SlotIndex <0 || SlotIndex >= PreviewMeshComponent->GetNumMaterials())
	{
		return false;
	}
	
	// Preview MIDを取得
	UMaterialInstanceDynamic* PreviewMID = Cast<UMaterialInstanceDynamic>(PreviewMeshComponent->GetMaterial(SlotIndex));
	
	if (!IsValid(PreviewMID))
	{
		return false;
	}
	
	// Base Color
	if (!BaseColorParameterName.IsNone())
	{
		PreviewMID->SetVectorParameterValue(
			BaseColorParameterName,
			BaseColor
		);	
	}
	
	// Shadow Color
	if (!ShadowColorParameterName.IsNone())
	{
		PreviewMID->SetVectorParameterValue(
			ShadowColorParameterName,
			ShadowColor
		);	
	}
	
	// Shadow Threshold
	if (!ShadowThresholdParameterName.IsNone())
	{
		PreviewMID->SetScalarParameterValue(
			ShadowThresholdParameterName,
			FMath::Clamp(ShadowThreshold, 0.0f, 1.0f)
		);	
	}
	
	// Viewport更新
	const TSharedPtr<FEditorViewportClient> ViewportClient = GetViewportClient();
	
	if (ViewportClient.IsValid())
	{
		ViewportClient->Invalidate(false, false);
	}
	Invalidate();
	return true;
}

// ==================================================
// プレビューTextureの更新
// ==================================================
bool SToonShaderPreviewViewport::SetPreviewTextureParameter(int32 SlotIndex,FName ParameterName,UTexture* Texture)
{
	// Preview Mesh確認
	if (!IsValid(PreviewMeshComponent))
	{
		return false;
	}

	// Parameter名確認
	if (ParameterName.IsNone())
	{
		return false;
	}

	// Slot範囲確認
	if (SlotIndex < 0 || SlotIndex >= PreviewMeshComponent->GetNumMaterials())
	{
		return false;
	}

	// ==========================================
	// Preview MID取得
	// ==========================================

	UMaterialInstanceDynamic* PreviewMID =
		Cast<UMaterialInstanceDynamic>(
			PreviewMeshComponent->GetMaterial(SlotIndex)
		);

	if (!IsValid(PreviewMID))
	{
		return false;
	}

	// ==========================================
	// Texture Parameter変更
	// ==========================================

	PreviewMID->SetTextureParameterValue(
		ParameterName,
		Texture
	);

	// ==========================================
	// 再描画
	// ==========================================

	const TSharedPtr<FEditorViewportClient> ViewportClient =
		GetViewportClient();

	if (ViewportClient.IsValid())
	{
		ViewportClient->Invalidate(false, false);
	}

	Invalidate();

	return true;
}
