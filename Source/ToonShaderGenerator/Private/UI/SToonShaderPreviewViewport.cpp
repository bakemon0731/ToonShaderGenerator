#include "UI/SToonShaderPreviewViewport.h"

#include "AdvancedPreviewScene.h"
#include "EditorViewportClient.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"


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
