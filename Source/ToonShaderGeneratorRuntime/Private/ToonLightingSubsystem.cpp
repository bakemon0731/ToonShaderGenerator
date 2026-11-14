
#include "Subsystems/ToonLightingSubsystem.h"

#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

#include "UObject/SoftObjectPath.h"
#include "Stats/Stats.h"


namespace
{
    // ==========================================
    // Toon Lighting設定
    // ==========================================

    const FName MainLightTag(TEXT("ToonMainLight"));

    const FName LightDirectionParameter(
        TEXT("DirectionalLightDirectionWS")
    );

    const FSoftObjectPath LightingCollectionPath(
        TEXT(
            "/ToonShaderGenerator/Materials/Parameters/"
            "MPC_ToonLighting.MPC_ToonLighting"
        )
    );

    constexpr float SearchRetryInterval = 1.0f;
    constexpr float DirectionChangeTolerance = 0.0001f;
}


// ==================================================
// World Subsystem生成条件
// ==================================================

bool UToonLightingSubsystem::ShouldCreateSubsystem(
    UObject* Outer) const
{
    const UWorld* World = Cast<UWorld>(Outer);

    if (!World)
    {
        return false;
    }

    return World->WorldType == EWorldType::Game
        || World->WorldType == EWorldType::PIE;
}


// ==================================================
// 初期化
// ==================================================

void UToonLightingSubsystem::Initialize(
    FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(
        LogTemp,
        Log,
        TEXT("ToonLightingSubsystem: Initialize")
    );


    // MPCを読み込む
    LightingCollection =
        Cast<UMaterialParameterCollection>(
            LightingCollectionPath.TryLoad()
        );

    if (!IsValid(LightingCollection))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("ToonLightingSubsystem: MPC_ToonLightingを読み込めません。")
        );

        return;
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("ToonLightingSubsystem: MPCを読み込みました。")
    );


    // 初回検索
    //TryFindDirectionalLight();
}


// ==================================================
// 終了処理
// ==================================================

void UToonLightingSubsystem::Deinitialize()
{
    CachedDirectionalLight.Reset();

    LightingCollection = nullptr;

    Super::Deinitialize();

    UE_LOG(
        LogTemp,
        Log,
        TEXT("ToonLightingSubsystem: Deinitialize")
    );
}


// ==================================================
// 毎フレーム処理
// ==================================================

void UToonLightingSubsystem::Tick(float DeltaTime)
{
    // MPCが読み込まれているか確認
    if (!IsValid(LightingCollection))
    {
        return;
    }


    // ==========================================
    //  Directional Lightを確認
    // ==========================================

    if (!CachedDirectionalLight.IsValid())
    {
        SearchRetryElapsed += DeltaTime;

        if (SearchRetryElapsed < SearchRetryInterval)
        {
            return;
        }

        SearchRetryElapsed = 0.0f;

        TryFindDirectionalLight();

        if (!CachedDirectionalLight.IsValid())
        {
            return;
        }
    }


    // ==========================================
    // Light Componentを取得
    // ==========================================

    ADirectionalLight* DirectionalLight =
        CachedDirectionalLight.Get();

    if (!IsValid(DirectionalLight))
    {
        CachedDirectionalLight.Reset();
        return;
    }

    UDirectionalLightComponent* LightComponent =
        DirectionalLight->GetComponent();

    if (!IsValid(LightComponent))
    {
        CachedDirectionalLight.Reset();
        return;
    }


    // ==========================================
    //  光の方向を取得
    // ==========================================

    // GetDirectionは光が進む方向
    const FVector LightRayDirection =
        LightComponent->GetDirection().GetSafeNormal();

    if (LightRayDirection.IsNearlyZero())
    {
        return;
    }

    // 顔の表面から光源へ向かう方向にする
    const FVector DirectionToLight =
        (-LightRayDirection).GetSafeNormal();


    // ==========================================
    //  方向が変わったか確認
    // ==========================================

    if (bHasSentDirection &&
        DirectionToLight.Equals(
            LastSentDirection,
            DirectionChangeTolerance
        ))
    {
        return;
    }


    // ==========================================
    // WorldのMPC Instanceを取得
    // ==========================================

    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        return;
    }

    UMaterialParameterCollectionInstance* CollectionInstance =
        World->GetParameterCollectionInstance(
            LightingCollection
        );

    if (!IsValid(CollectionInstance))
    {
        if (!bLoggedMissingCollectionInstance)
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT("ToonLightingSubsystem: MPC Instanceを取得できません。")
            );

            bLoggedMissingCollectionInstance = true;
        }

        return;
    }


    // ==========================================
    //  MPCを更新
    // ==========================================

    const bool bUpdated =
        CollectionInstance->SetVectorParameterValue(
            LightDirectionParameter,
            DirectionToLight
        );

    if (!bUpdated)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("ToonLightingSubsystem: MPCのVector Parameter更新に失敗しました。")
        );

        return;
    }


    // ==========================================
    // 最終値を保存
    // ==========================================

    LastSentDirection = DirectionToLight;
    bHasSentDirection = true;

    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "ToonLightingSubsystem: Direction Updated "
            "X=%.3f Y=%.3f Z=%.3f"
        ),
        DirectionToLight.X,
        DirectionToLight.Y,
        DirectionToLight.Z
    );
}


// ==================================================
// Directional Lightを検索
// ==================================================

void UToonLightingSubsystem::TryFindDirectionalLight()
{
    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        return;
    }

    ADirectionalLight* FoundLight = nullptr;

    int32 MatchingLightCount = 0;


    // World内のDirectional Lightを検索
    for (TActorIterator<ADirectionalLight> It(World);
         It;
         ++It)
    {
        if (!It->ActorHasTag(MainLightTag))
        {
            continue;
        }

        ++MatchingLightCount;

        FoundLight = *It;
    }


    // ==========================================
    //  見つからない場合
    // ==========================================

    if (MatchingLightCount == 0)
    {
        if (!bLoggedMissingLight)
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT(
                    "ToonLightingSubsystem: ToonMainLightタグを持つ"
                    "Directional Lightがありません。"
                )
            );

            bLoggedMissingLight = true;
        }

        return;
    }


    // ==========================================
    // 複数存在する場合
    // ==========================================

    if (MatchingLightCount > 1)
    {
        CachedDirectionalLight.Reset();

        if (!bLoggedMultipleLights)
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT(
                    "ToonLightingSubsystem: ToonMainLightタグを持つ"
                    "Directional Lightが複数あります。1つだけ指定してください。"
                )
            );

            bLoggedMultipleLights = true;
        }

        return;
    }


    // ==========================================
    // 正常に見つかった場合
    // ==========================================

    CachedDirectionalLight = FoundLight;

    bLoggedMissingLight = false;
    bLoggedMultipleLights = false;

    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "ToonLightingSubsystem: Main Directional Light Found: %s"
        ),
        *FoundLight->GetName()
    );
}


// ==================================================
// Tick統計情報
// ==================================================

TStatId UToonLightingSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(
        UToonLightingSubsystem,
        STATGROUP_Tickables
    );
}
