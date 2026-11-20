#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"

// 前方宣言
class FAdvancedPreviewScene;
class USkeletalMesh;
class USkeletalMeshComponent;
class UMaterialInterface;

class SToonShaderPreviewViewport : public SEditorViewport
{
public:

	SLATE_BEGIN_ARGS(SToonShaderPreviewViewport)
	{}
	SLATE_END_ARGS()

	// ウィジェット初期化
	void Construct(const FArguments& InArgs);
	
	// プレビューするSkeletal Meshを設定
	void SetPreviewSkeletalMesh(USkeletalMesh* NewMesh);
	
	// カメラを初期視点に戻す
	void ResetCamera();
	
	// プレビュー専用Materialを設定
	bool SetPreviewMaterialForSlot(int32 SlotIndex, UMaterialInterface* SourceMaterial);
	
	// プレビューのMaterialを元の状態に戻す
	bool RestorePreviewMaterialForSlot(int32 SlotIndex);

protected:

	// Viewport Client生成
	virtual TSharedRef<FEditorViewportClient>
		MakeEditorViewportClient() override;

private:

	// プレビュー専用の3D空間
	TSharedPtr<FAdvancedPreviewScene> PreviewScene;
	
	// プレビュー表示用コンポーネント
	USkeletalMeshComponent* PreviewMeshComponent = nullptr;
};