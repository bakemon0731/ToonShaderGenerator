
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


    if (Settings.MaterialType == EToonMaterialType::Hair)
    {
        // Flow Mapが指定されている場合だけロード
        if (!Settings.HairFlowMapTexturePath.IsNull())
        {
            HairFlowMapTexture = Cast<UTexture2D>(
                Settings.HairFlowMapTexturePath.TryLoad()
            );

            if (!IsValid(HairFlowMapTexture))
            {
                OutErrorMessage =
                    TEXT("Hair Flow Mapを読み込めません。");

                return nullptr;
            }
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

        // 髪の基本色
        CreatedInstance->SetVectorParameterValueEditorOnly(
            FMaterialParameterInfo(FName(TEXT("HairBaseColor"))),
            Settings.Basic.BaseColor
        );

        // Roughness
        CreatedInstance->SetScalarParameterValueEditorOnly(
            FMaterialParameterInfo(FName(TEXT("HairRoughness"))),
            Settings.HairRoughness
        );

        // Specular
        CreatedInstance->SetScalarParameterValueEditorOnly(
            FMaterialParameterInfo(FName(TEXT("HairSpecular"))),
            Settings.HairSpecular
        );

        // Anisotropy
        CreatedInstance->SetScalarParameterValueEditorOnly(
            FMaterialParameterInfo(FName(TEXT("HairAnisotropy"))),
            Settings.HairAnisotropy
        );

        // Flow Mapが存在する場合だけ設定
        if (IsValid(HairFlowMapTexture))
        {
            CreatedInstance->SetTextureParameterValueEditorOnly(
                FMaterialParameterInfo(FName(TEXT("HairFlowMap"))),
                HairFlowMapTexture
            );
        }

        // Static Switch
        // Flow Mapあり：True
        // Flow Mapなし：False
        CreatedInstance->SetStaticSwitchParameterValueEditorOnly(
            FMaterialParameterInfo(FName(TEXT("UseHairFlowMap"))),
            IsValid(HairFlowMapTexture)
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
