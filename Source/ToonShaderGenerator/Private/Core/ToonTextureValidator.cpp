
#include "Core/ToonTextureValidator.h"

#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureDefines.h"


namespace
{
    // ==========================================
    // 検証結果を追加
    // ==========================================

    void AddValidationIssue(
        const FToonMaterialSlotInfo& SlotInfo,
        const FString& InputName,
        EToonValidationSeverity Severity,
        const FString& Message,
        TArray<FToonValidationIssue>& OutIssues)
    {
        FToonValidationIssue Issue;

        Issue.SlotIndex = SlotInfo.SlotIndex;
        Issue.SlotName = SlotInfo.SlotName;
        Issue.InputName = InputName;
        Issue.Severity = Severity;
        Issue.Message = FText::FromString(Message);

        OutIssues.Add(MoveTemp(Issue));
    }


    // ==========================================
    // テクスチャ1件を検証
    // ==========================================

    void ValidateTextureInput(
        const FToonMaterialSlotInfo& SlotInfo,
        const FString& InputName,
        const FSoftObjectPath& TexturePath,
        bool bRequired,
        bool bExpectedSRGB,
        bool bCheckCompression,
        TextureCompressionSettings ExpectedCompression,
        const FString& ExpectedCompressionName,
        TArray<FToonValidationIssue>& OutIssues)
    {
        // --------------------------------------
        //  未設定チェック
        // --------------------------------------

        if (TexturePath.IsNull())
        {
            if (bRequired)
            {
                AddValidationIssue(
                    SlotInfo,
                    InputName,
                    EToonValidationSeverity::Warning,
                    FString::Printf(
                        TEXT("%sが設定されていません。"),
                        *InputName
                    ),
                    OutIssues
                );
            }

            return;
        }


        // --------------------------------------
        // アセット読み込み
        // --------------------------------------

        UObject* LoadedObject = TexturePath.TryLoad();

        if (!IsValid(LoadedObject))
        {
            AddValidationIssue(
                SlotInfo,
                InputName,
                EToonValidationSeverity::Error,
                FString::Printf(
                    TEXT("%sを読み込めません。参照先を確認してください。"),
                    *InputName
                ),
                OutIssues
            );

            return;
        }


        // --------------------------------------
        //  Texture2Dか確認
        // --------------------------------------

        UTexture2D* Texture = Cast<UTexture2D>(LoadedObject);

        if (!IsValid(Texture))
        {
            AddValidationIssue(
                SlotInfo,
                InputName,
                EToonValidationSeverity::Error,
                FString::Printf(
                    TEXT("%sがTexture2Dではありません。"),
                    *InputName
                ),
                OutIssues
            );

            return;
        }


        // --------------------------------------
        // sRGB設定を確認
        // --------------------------------------

        const bool bActualSRGB = Texture->SRGB != 0;

        if (bActualSRGB != bExpectedSRGB)
        {
            const FString ExpectedValue =
                bExpectedSRGB ? TEXT("ON") : TEXT("OFF");

            AddValidationIssue(
                SlotInfo,
                InputName,
                EToonValidationSeverity::Warning,
                FString::Printf(
                    TEXT(
                        "%sのsRGB設定が推奨値と異なります。"
                        "推奨設定：%s"
                    ),
                    *InputName,
                    *ExpectedValue
                ),
                OutIssues
            );
        }


        // --------------------------------------
        // Compression Settingsを確認
        // --------------------------------------

        if (bCheckCompression &&
            Texture->CompressionSettings != ExpectedCompression)
        {
            AddValidationIssue(
                SlotInfo,
                InputName,
                EToonValidationSeverity::Warning,
                FString::Printf(
                    TEXT(
                        "%sのCompression Settingsが"
                        "推奨設定と異なります。推奨設定：%s"
                    ),
                    *InputName,
                    *ExpectedCompressionName
                ),
                OutIssues
            );
        }
    }
}


// ==================================================
// Material Slotの検証
// ==================================================

void FToonTextureValidator::ValidateSlot(
    const FToonMaterialSlotInfo& SlotInfo,
    TArray<FToonValidationIssue>& OutIssues)
{
    switch (SlotInfo.MaterialType)
    {
    // ==========================================
    // Face
    // ==========================================

    case EToonMaterialType::Face:

        ValidateTextureInput(
            SlotInfo,
            TEXT("Face SDF"),
            SlotInfo.FaceSDFTexturePath,
            true,
            false,
            false,
            TC_Default,
            TEXT(""),
            OutIssues
        );

        break;


    // ==========================================
    // Hair
    // ==========================================

    case EToonMaterialType::Hair:

        ValidateTextureInput(
            SlotInfo,
            TEXT("Hair Flow Map"),
            SlotInfo.HairFlowMapTexturePath,
            true,
            false,
            true,
            TC_Normalmap,
            TEXT("Normalmap"),
            OutIssues
        );

        break;


    // ==========================================
    // Skin
    // ==========================================

    case EToonMaterialType::Skin:

        ValidateTextureInput(
            SlotInfo,
            TEXT("Skin Base Color"),
            SlotInfo.SkinBaseColorTexturePath,
            false,
            true,
            false,
            TC_Default,
            TEXT("Default"),
            OutIssues
        );

        break;


    // ==========================================
    // Clothing
    // ==========================================

    case EToonMaterialType::Clothing:

        // Base Color
        ValidateTextureInput(
            SlotInfo,
            TEXT("Clothing Base Color"),
            SlotInfo.ClothingBaseColorTexturePath,
            false,
            true,
            false,
            TC_Default,
            TEXT("Default"),
            OutIssues
        );

        // Normal
        ValidateTextureInput(
            SlotInfo,
            TEXT("Clothing Normal"),
            SlotInfo.ClothingNormalTexturePath,
            false,
            false,
            true,
            TC_Normalmap,
            TEXT("Normalmap"),
            OutIssues
        );

        // ORM
        ValidateTextureInput(
            SlotInfo,
            TEXT("Clothing ORM"),
            SlotInfo.ClothingORMTexturePath,
            false,
            false,
            true,
            TC_Masks,
            TEXT("Masks"),
            OutIssues
        );

        break;


    // ==========================================
    // Other
    // ==========================================

    case EToonMaterialType::Other:
    default:
        break;
    }
}
