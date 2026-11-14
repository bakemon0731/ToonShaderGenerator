#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ToonLightingSubsystem.generated.h"

class ADirectionalLight;
class UMaterialParameterCollection;

UCLASS()
class TOONSHADERGENERATORRUNTIME_API UToonLightingSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
public:
	
	// Subsystemの初期化
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	// Subsystemの終了処理
	virtual void Deinitialize() override;
	
	// 毎フレーム実行
	virtual void Tick(float DeltaTime) override;
	
	// Tickの統計情報
	virtual TStatId GetStatId() const override;
	
	// 対象Worldを限定
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
private:
	
	// Tagを使ってDirectional Lightを探す
	void TryFindDirectionalLight();
	
private:
	
	// MPC
	UPROPERTY(Transient)
	TObjectPtr<UMaterialParameterCollection> LightingCollection;
	
	// 取得したDirectional Light
	TWeakObjectPtr<ADirectionalLight> CachedDirectionalLight;
	
	// 前回MPCに送信した方向
	FVector LastSentDirection = FVector::ZeroVector;
	
	// 初回送信済みか
	bool bHasSentDirection = false;
	
	// 光源を再検索する間隔
	float SearchRetryElapsed = 0.0f;
	
	// ログの重複防止
	bool bLoggedMissingLight = false;
	bool bLoggedMultipleLights = false;
	bool bLoggedMissingCollectionInstance = false;
};