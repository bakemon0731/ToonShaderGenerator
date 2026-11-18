
#include "Core/ToonMaterialInstanceCreator.h"

#include "AssetToolsModule.h"
#include "IAssetTools.h"

#include "Factories/MaterialInstanceConstantFactoryNew.h"

#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInterface.h"

#include "Engine/Texture2D.h"

#include "Misc/PackageName.h"


UMaterialInstanceConstant*
FToonMaterialInstanceCreator::Create(
    UMaterialInterface* ParentMaterial,
    const FString& AssetName,
    const FString& PackagePath,
    const FToonMaterialInstanceSettings& Settings,
    FString& OutErrorMessage)
{
    OutErrorMessage.Reset();


    // ==========================================
    // 1. 入力検証
    // ==========================================

    if (!IsValid(ParentMaterial))
    {
        OutErrorMessage =
            TEXT("親となるMaterialが無効です。");

        return nullptr;
    }

    if (AssetName.IsEmpty())
    {
        OutErrorMessage =
            TEXT("アセット名が指定されていません。");

        return nullptr;
    }

    if (PackagePath.IsEmpty())
    {
        OutErrorMessage =
            TEXT("保存先フォルダが指定されていません。");

        return nullptr;
    }


    // ==========================================
    // 2. Texture変数
    // ==========================================

    UTexture2D* FaceSDFTexture = nullptr;

    UTexture2D* FaceBaseColorTexture = nullptr;

    UTexture2D* LoadedHairFlowMap = nullptr;

    UTexture2D* LoadedSkinBaseColorTexture = nullptr;

    UTexture2D* LoadedClothingBaseColorTexture = nullptr;

    UTexture2D* LoadedClothingNormalTexture = nullptr;

    UTexture2D* LoadedClothingORMTexture = nullptr;


    // ==========================================
    // 3. Face Texture Load
    // ==========================================

    if (Settings.MaterialType == EToonMaterialType::Face)
    {
        // --------------------------------------
        // Face SDF
        // --------------------------------------

        if (!Settings.FaceSDFTexturePath.IsNull())
        {
            FaceSDFTexture = Cast<UTexture2D>(
                Settings.FaceSDFTexturePath.TryLoad()
            );

            if (!IsValid(FaceSDFTexture))
            {
                OutErrorMessage =
                    TEXT("Face SDF Textureを読み込めません。");

                return nullptr;
            }
        }


        // --------------------------------------
        // Face Base Color
        // --------------------------------------

        if (!Settings.FaceBaseColorTexturePath.IsNull())
        {
            FaceBaseColorTexture = Cast<UTexture2D>(
                Settings.FaceBaseColorTexturePath.TryLoad()
            );

            if (!IsValid(FaceBaseColorTexture))
            {
                OutErrorMessage =
                    TEXT("Face Base Color Textureを読み込めません。");

                return nullptr;
            }
        }


        // --------------------------------------
        // Required Texture Check
        // --------------------------------------

        // FaceSDFはマスターマテリアルに標準SDFが固定で入っているため、
        // 未指定でもよい(指定された場合のみ上書きする)
    }


    // ==========================================
    // 4. Hair Texture Load
    // ==========================================

    if (Settings.MaterialType == EToonMaterialType::Hair
        && Settings.bUseHairFlowMap)
    {
        if (Settings.HairFlowMapTexturePath.IsNull())
        {
            OutErrorMessage = TEXT(
                "Flow Mapが有効ですが、テクスチャが設定されていません。"
            );

            return nullptr;
        }

        LoadedHairFlowMap = Cast<UTexture2D>(
            Settings.HairFlowMapTexturePath.TryLoad()
        );

        if (!IsValid(LoadedHairFlowMap))
        {
            OutErrorMessage = TEXT(
                "Hair Flow Mapの読み込みに失敗しました。"
            );

            return nullptr;
        }
    }


    // ==========================================
    // 5. Skin Texture Load
    // ==========================================

    if (Settings.MaterialType == EToonMaterialType::Skin)
    {
        if (Settings.SkinBaseColorTexturePath.IsNull())
        {
            OutErrorMessage = TEXT(
                "Skin BaseColor Textureが設定されていません。"
            );

            return nullptr;
        }

        LoadedSkinBaseColorTexture =
            Cast<UTexture2D>(
                Settings.SkinBaseColorTexturePath.TryLoad()
            );

        if (!IsValid(LoadedSkinBaseColorTexture))
        {
            OutErrorMessage = TEXT(
                "Skin BaseColor Textureの読み込みに失敗しました。"
            );

            return nullptr;
        }
    }


    // ==========================================
    // 6. Clothing Texture Load
    // ==========================================

    if (Settings.MaterialType == EToonMaterialType::Clothing)
    {
        // --------------------------------------
        // BaseColor
        // --------------------------------------

        if (!Settings.ClothingBaseColorTexturePath.IsNull())
        {
            LoadedClothingBaseColorTexture =
                Cast<UTexture2D>(
                    Settings.ClothingBaseColorTexturePath.TryLoad()
                );

            if (!IsValid(LoadedClothingBaseColorTexture))
            {
                OutErrorMessage = TEXT(
                    "Clothing BaseColor Textureの読み込みに失敗しました。"
                );

                return nullptr;
            }
        }


        // --------------------------------------
        // Normal
        // --------------------------------------

        if (!Settings.ClothingNormalTexturePath.IsNull())
        {
            LoadedClothingNormalTexture =
                Cast<UTexture2D>(
                    Settings.ClothingNormalTexturePath.TryLoad()
                );

            if (!IsValid(LoadedClothingNormalTexture))
            {
                OutErrorMessage = TEXT(
                    "Clothing Normal Textureの読み込みに失敗しました。"
                );

                return nullptr;
            }
        }


        // --------------------------------------
        // ORM
        // --------------------------------------

        if (!Settings.ClothingORMTexturePath.IsNull())
        {
            LoadedClothingORMTexture =
                Cast<UTexture2D>(
                    Settings.ClothingORMTexturePath.TryLoad()
                );

            if (!IsValid(LoadedClothingORMTexture))
            {
                OutErrorMessage = TEXT(
                    "Clothing ORM Textureの読み込みに失敗しました。"
                );

                return nullptr;
            }
        }
    }


    // ==========================================
    // 7. Package Name Validation
    // ==========================================

    const FString BasePackageName =
        FString::Printf(
            TEXT("%s/%s"),
            *PackagePath,
            *AssetName
        );

    if (!FPackageName::IsValidLongPackageName(BasePackageName))
    {
        OutErrorMessage =
            TEXT("アセット名または保存先パスが不正です。");

        return nullptr;
    }


    // ==========================================
    // 8. AssetTools取得
    // ==========================================

    IAssetTools& AssetTools =
        FAssetToolsModule::GetModule().Get();


    // ==========================================
    // 9. 重複しない名前を作成
    // ==========================================

    FString UniquePackageName;
    FString UniqueAssetName;

    AssetTools.CreateUniqueAssetName(
        BasePackageName,
        TEXT("_"),
        UniquePackageName,
        UniqueAssetName
    );

    const FString UniquePackagePath =
        FPackageName::GetLongPackagePath(
            UniquePackageName
        );


    // ==========================================
    // 10. Factory生成
    // ==========================================

    UMaterialInstanceConstantFactoryNew* Factory =
        NewObject<UMaterialInstanceConstantFactoryNew>();

    Factory->InitialParent = ParentMaterial;


    // ==========================================
    // 11. Material Instance生成
    // ==========================================

    UObject* CreatedObject = AssetTools.CreateAsset(
        UniqueAssetName,
        UniquePackagePath,
        UMaterialInstanceConstant::StaticClass(),
        Factory,
        FName(TEXT("ToonShaderGenerator"))
    );

    UMaterialInstanceConstant* CreatedInstance =
        Cast<UMaterialInstanceConstant>(CreatedObject);

    if (!IsValid(CreatedInstance))
    {
        OutErrorMessage =
            TEXT("Material Instanceの生成に失敗しました。");

        return nullptr;
    }


    // ==========================================
    // 12. Material Parameter設定
    // ==========================================

    switch (Settings.MaterialType)
    {

    // ==========================================
    // Basic / Other
    // ==========================================

    case EToonMaterialType::Other:

        CreatedInstance->SetVectorParameterValueEditorOnly(
            FMaterialParameterInfo(
                FName(TEXT("BaseColor"))
            ),
            Settings.Basic.BaseColor
        );

        CreatedInstance->SetVectorParameterValueEditorOnly(
            FMaterialParameterInfo(
                FName(TEXT("ShadowColor"))
            ),
            Settings.Basic.ShadowColor
        );

        CreatedInstance->SetScalarParameterValueEditorOnly(
            FMaterialParameterInfo(
                FName(TEXT("ShadowThreshold"))
            ),
            FMath::Clamp(
                Settings.Basic.ShadowThreshold,
                0.0f,
                1.0f
            )
        );

        break;


    // ==========================================
    // Face Shader
    // ==========================================

    case EToonMaterialType::Face:
        {
            // --------------------------------------
            // Basic Color
            // --------------------------------------

            CreatedInstance->SetVectorParameterValueEditorOnly(
                FMaterialParameterInfo(
                    FName(TEXT("BaseColor"))
                ),
                Settings.Basic.BaseColor
            );

            CreatedInstance->SetVectorParameterValueEditorOnly(
                FMaterialParameterInfo(
                    FName(TEXT("ShadowColor"))
                ),
                Settings.Basic.ShadowColor
            );


            // --------------------------------------
            // Face SDF
            // --------------------------------------

            // 指定されている場合のみ、固定の標準SDFを入れ替える
            if (IsValid(FaceSDFTexture))
            {
                CreatedInstance->SetTextureParameterValueEditorOnly(
                    FMaterialParameterInfo(
                        FName(TEXT("FaceSDFTexture"))
                    ),
                    FaceSDFTexture
                );
            }


            // --------------------------------------
            // Face Base Color Texture
            // --------------------------------------

            if (IsValid(FaceBaseColorTexture))
            {
                CreatedInstance->SetTextureParameterValueEditorOnly(
                    FMaterialParameterInfo(
                        FName(TEXT("FaceBaseColorTexture"))
                    ),
                    FaceBaseColorTexture
                );
            }


            // FaceSDFをサンプルするUVのV方向の位置・大きさ
            CreatedInstance->SetScalarParameterValueEditorOnly(
                FMaterialParameterInfo(
                    FName(TEXT("FaceSDFVOffset"))
                ),
                Settings.FaceSDFVOffset
            );

            CreatedInstance->SetScalarParameterValueEditorOnly(
                FMaterialParameterInfo(
                    FName(TEXT("FaceSDFVScale"))
                ),
                FMath::Max(Settings.FaceSDFVScale, 0.01f)
            );

            CreatedInstance->SetScalarParameterValueEditorOnly(
                FMaterialParameterInfo(
                    FName(TEXT("FaceBoundarySoftness"))
                ),
                FMath::Max(
                    Settings.FaceBoundarySoftness,
                    0.0f
                )
            );


            // --------------------------------------
            // Face SDF Mode / Axes
            // --------------------------------------

            // FaceSDFは固定・任意指定のどちらも1チャンネルSDF
            // (UVのU反転で左右を切り替える)なので常にtrue
            CreatedInstance->SetStaticSwitchParameterValueEditorOnly(
                FMaterialParameterInfo(
                    FName(TEXT("FaceSDFSingleChannelMirror"))
                ),
                true
            );

            // 顔のローカル空間での前方向・右方向
            CreatedInstance->SetVectorParameterValueEditorOnly(
                FMaterialParameterInfo(
                    FName(TEXT("FaceForwardAxis"))
                ),
                FLinearColor(
                    Settings.FaceForwardAxis.X,
                    Settings.FaceForwardAxis.Y,
                    Settings.FaceForwardAxis.Z,
                    0.0f
                )
            );

            CreatedInstance->SetVectorParameterValueEditorOnly(
                FMaterialParameterInfo(
                    FName(TEXT("FaceRightAxis"))
                ),
                FLinearColor(
                    Settings.FaceRightAxis.X,
                    Settings.FaceRightAxis.Y,
                    Settings.FaceRightAxis.Z,
                    0.0f
                )
            );


            break;
        }    


    // ==========================================
    // Hair Shader
    // ==========================================

    case EToonMaterialType::Hair:

        // Base Color
        CreatedInstance->SetVectorParameterValueEditorOnly(
            FMaterialParameterInfo(
                FName(TEXT("HairBaseColor"))
            ),
            Settings.Basic.BaseColor
        );


        // Roughness
        CreatedInstance->SetScalarParameterValueEditorOnly(
            FMaterialParameterInfo(
                FName(TEXT("HairRoughness"))
            ),
            FMath::Clamp(
                Settings.HairRoughness,
                0.0f,
                1.0f
            )
        );


        // Specular
        CreatedInstance->SetScalarParameterValueEditorOnly(
            FMaterialParameterInfo(
                FName(TEXT("HairSpecular"))
            ),
            FMath::Clamp(
                Settings.HairSpecular,
                0.0f,
                1.0f
            )
        );


        // Anisotropy
        CreatedInstance->SetScalarParameterValueEditorOnly(
            FMaterialParameterInfo(
                FName(TEXT("HairAnisotropy"))
            ),
            FMath::Clamp(
                Settings.HairAnisotropy,
                0.0f,
                1.0f
            )
        );


        // Flow Map Texture
        if (Settings.bUseHairFlowMap
            && IsValid(LoadedHairFlowMap))
        {
            CreatedInstance->SetTextureParameterValueEditorOnly(
                FMaterialParameterInfo(
                    FName(TEXT("HairFlowMap"))
                ),
                LoadedHairFlowMap
            );
        }


        // Static Switch
        CreatedInstance->SetStaticSwitchParameterValueEditorOnly(
            FMaterialParameterInfo(
                FName(TEXT("UseHairFlowMap"))
            ),
            Settings.bUseHairFlowMap
        );

        break;


    // ==========================================
    // Skin Shader
    // ==========================================

    case EToonMaterialType::Skin:

        // Base Color Texture
        CreatedInstance->SetTextureParameterValueEditorOnly(
            FMaterialParameterInfo(
                FName(TEXT("SkinBaseColorTexture"))
            ),
            LoadedSkinBaseColorTexture
        );


        // Skin Tint
        CreatedInstance->SetVectorParameterValueEditorOnly(
            FMaterialParameterInfo(
                FName(TEXT("SkinTint"))
            ),
            Settings.Basic.BaseColor
        );


        // Shadow Color
        CreatedInstance->SetVectorParameterValueEditorOnly(
            FMaterialParameterInfo(
                FName(TEXT("SkinShadowColor"))
            ),
            Settings.Basic.ShadowColor
        );


        // Shadow Threshold
        CreatedInstance->SetScalarParameterValueEditorOnly(
            FMaterialParameterInfo(
                FName(TEXT("SkinShadowThreshold"))
            ),
            FMath::Clamp(
                Settings.Basic.ShadowThreshold,
                0.0f,
                1.0f
            )
        );

        break;


    // ==========================================
    // Clothing Shader
    // ==========================================

    case EToonMaterialType::Clothing:

        // --------------------------------------
        // Base Color Texture
        // --------------------------------------

        if (IsValid(LoadedClothingBaseColorTexture))
        {
            CreatedInstance->SetTextureParameterValueEditorOnly(
                FMaterialParameterInfo(
                    FName(TEXT("ClothingBaseColorTexture"))
                ),
                LoadedClothingBaseColorTexture
            );
        }


        // --------------------------------------
        // Normal Texture
        // --------------------------------------

        if (IsValid(LoadedClothingNormalTexture))
        {
            CreatedInstance->SetTextureParameterValueEditorOnly(
                FMaterialParameterInfo(
                    FName(TEXT("ClothingNormalTexture"))
                ),
                LoadedClothingNormalTexture
            );
        }


        // Normal Map Static Switch
        CreatedInstance->SetStaticSwitchParameterValueEditorOnly(
            FMaterialParameterInfo(
                FName(TEXT("UseClothingNormalMap"))
            ),
            IsValid(LoadedClothingNormalTexture)
        );


        // --------------------------------------
        // ORM Texture
        // --------------------------------------

        if (IsValid(LoadedClothingORMTexture))
        {
            CreatedInstance->SetTextureParameterValueEditorOnly(
                FMaterialParameterInfo(
                    FName(TEXT("ClothingORMTexture"))
                ),
                LoadedClothingORMTexture
            );
        }


        // --------------------------------------
        // Clothing Tint
        // --------------------------------------

        CreatedInstance->SetVectorParameterValueEditorOnly(
            FMaterialParameterInfo(
                FName(TEXT("ClothingTint"))
            ),
            Settings.Basic.BaseColor
        );


        // --------------------------------------
        // Shadow Color
        // --------------------------------------

        CreatedInstance->SetVectorParameterValueEditorOnly(
            FMaterialParameterInfo(
                FName(TEXT("ClothingShadowColor"))
            ),
            Settings.Basic.ShadowColor
        );


        // --------------------------------------
        // Shadow Threshold
        // --------------------------------------

        CreatedInstance->SetScalarParameterValueEditorOnly(
            FMaterialParameterInfo(
                FName(TEXT("ClothingShadowThreshold"))
            ),
            FMath::Clamp(
                Settings.Basic.ShadowThreshold,
                0.0f,
                1.0f
            )
        );

        break;


    // ==========================================
    // Unsupported Material Type
    // ==========================================

    default:

        OutErrorMessage =
            TEXT("未対応のMaterial Typeです。");

        return nullptr;
    }


    // ==========================================
    // 13. 変更を反映
    // ==========================================

    CreatedInstance->PostEditChange();

    CreatedInstance->MarkPackageDirty();

    return CreatedInstance;
}
