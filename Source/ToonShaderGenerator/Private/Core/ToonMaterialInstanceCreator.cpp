
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
    // 入力検証
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
    //  必要なTextureをロード
    // ==========================================

    UTexture2D* FaceSDFTexture = nullptr;
    UTexture2D* HairFlowMapTexture = nullptr;
    UTexture2D* LoadedHairFlowMap = nullptr;
    UTexture2D* LoadedSkinBaseColorTexture = nullptr;


    if (Settings.MaterialType == EToonMaterialType::Face)
    {
        if (Settings.FaceSDFTexturePath.IsNull())
        {
            OutErrorMessage =
                TEXT("Face SDF Textureが指定されていません。");

            return nullptr;
        }

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

        if (!LoadedHairFlowMap)
        {
            OutErrorMessage = TEXT(
                "Hair Flow Mapの読み込みに失敗しました。"
            );

            return nullptr;
        }
    }
    
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

        if (!LoadedSkinBaseColorTexture)
        {
            OutErrorMessage = TEXT(
                "Skin BaseColor Textureの読み込みに失敗しました。"
            );

            return nullptr;
        }
    }


    // ==========================================
    // Package Name検証
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
    // AssetTools取得
    // ==========================================

    IAssetTools& AssetTools =
        FAssetToolsModule::GetModule().Get();


    // ==========================================
    //  重複しない名前を作成
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
    //  Factory生成
    // ==========================================

    UMaterialInstanceConstantFactoryNew* Factory =
        NewObject<UMaterialInstanceConstantFactoryNew>();

    Factory->InitialParent = ParentMaterial;


    // ==========================================
    //  Material Instance生成
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
    //  Material Parameter設定
    // ==========================================

    switch (Settings.MaterialType)
    {

    // ==========================================
    // Basic / Other
    // ==========================================

    case EToonMaterialType::Other:

        CreatedInstance->SetVectorParameterValueEditorOnly(
            FMaterialParameterInfo(FName(TEXT("BaseColor"))),
            Settings.Basic.BaseColor
        );

        CreatedInstance->SetVectorParameterValueEditorOnly(
            FMaterialParameterInfo(FName(TEXT("ShadowColor"))),
            Settings.Basic.ShadowColor
        );

        CreatedInstance->SetScalarParameterValueEditorOnly(
            FMaterialParameterInfo(FName(TEXT("ShadowThreshold"))),
            Settings.Basic.ShadowThreshold
        );

        break;


    // ==========================================
    // Face Shader
    // ==========================================

    case EToonMaterialType::Face:

        CreatedInstance->SetVectorParameterValueEditorOnly(
            FMaterialParameterInfo(FName(TEXT("BaseColor"))),
            Settings.Basic.BaseColor
        );

        CreatedInstance->SetVectorParameterValueEditorOnly(
            FMaterialParameterInfo(FName(TEXT("ShadowColor"))),
            Settings.Basic.ShadowColor
        );

        CreatedInstance->SetTextureParameterValueEditorOnly(
            FMaterialParameterInfo(FName(TEXT("FaceSDFTexture"))),
            FaceSDFTexture
        );

        CreatedInstance->SetScalarParameterValueEditorOnly(
            FMaterialParameterInfo(FName(TEXT("FaceBoundarySoftness"))),
            Settings.FaceBoundarySoftness
        );

        break;


    // ==========================================
    // Hair Shader
    // ==========================================

    case EToonMaterialType::Hair:

        // Base Color
        CreatedInstance->SetVectorParameterValueEditorOnly(
            FMaterialParameterInfo(TEXT("HairBaseColor")),
            Settings.Basic.BaseColor
        );

        // Roughness
        CreatedInstance->SetScalarParameterValueEditorOnly(
            FMaterialParameterInfo(TEXT("HairRoughness")),
            FMath::Clamp(Settings.HairRoughness, 0.0f, 1.0f)
        );

        // Specular
        CreatedInstance->SetScalarParameterValueEditorOnly(
            FMaterialParameterInfo(TEXT("HairSpecular")),
            FMath::Clamp(Settings.HairSpecular, 0.0f, 1.0f)
        );

        // Anisotropy
        CreatedInstance->SetScalarParameterValueEditorOnly(
            FMaterialParameterInfo(TEXT("HairAnisotropy")),
            FMath::Clamp(Settings.HairAnisotropy, 0.0f, 1.0f)
        );

        // Flow Map Texture
        if (Settings.bUseHairFlowMap && LoadedHairFlowMap)
        {
            CreatedInstance->SetTextureParameterValueEditorOnly(
                FMaterialParameterInfo(TEXT("HairFlowMap")),
                LoadedHairFlowMap
            );
        }

        // Static Switch
        CreatedInstance->SetStaticSwitchParameterValueEditorOnly(
            FMaterialParameterInfo(TEXT("UseHairFlowMap")),
            Settings.bUseHairFlowMap
        );

        break;

    // ==========================================
    // Hair Shader
    // ========================================== 
    case EToonMaterialType::Skin:
        
        // Base Color Texture
        CreatedInstance->SetTextureParameterValueEditorOnly(
            FMaterialParameterInfo(TEXT("SkinBaseColorTexture")),
            LoadedSkinBaseColorTexture
        );

        // Skin Tint
        CreatedInstance->SetVectorParameterValueEditorOnly(
            FMaterialParameterInfo(TEXT("SkinTint")),
            Settings.Basic.BaseColor
        );

        // Shadow Color
        CreatedInstance->SetVectorParameterValueEditorOnly(
            FMaterialParameterInfo(TEXT("SkinShadowColor")),
            Settings.Basic.ShadowColor
        );

        // Shadow Threshold
        CreatedInstance->SetScalarParameterValueEditorOnly(
            FMaterialParameterInfo(TEXT("SkinShadowThreshold")),
            FMath::Clamp(
                Settings.Basic.ShadowThreshold,
                0.0f,
                1.0f
            )
        );
        
        break;

    default:

        OutErrorMessage =
            TEXT("未対応のMaterial Typeです。");

        return nullptr;
    }


    // ==========================================
    // 変更を反映
    // ==========================================

    CreatedInstance->PostEditChange();

    CreatedInstance->MarkPackageDirty();

    return CreatedInstance;
}
