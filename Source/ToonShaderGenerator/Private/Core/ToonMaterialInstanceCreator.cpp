#include "Core/ToonMaterialInstanceCreator.h"

#include "AssetToolsModule.h"
#include "IAssetTools.h"

#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInterface.h"

#include "Misc/PackageName.h"


UMaterialInstanceConstant*
FToonMaterialInstanceCreator::Create(
    UMaterialInterface* ParentMaterial,
    const FString& AssetName,
    const FString& PackagePath,
    FString& OutErrorMessage)
{
    OutErrorMessage.Reset();

    // ==========================================
    //  入力検証
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

    // アセットの完全なパッケージ名
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
    // AssetToolsを取得
    // ==========================================

    IAssetTools& AssetTools =
        FAssetToolsModule::GetModule().Get();


    // ==========================================
    // 重複しないアセット名を生成
    // ==========================================

    FString UniquePackageName;
    FString UniqueAssetName;

    AssetTools.CreateUniqueAssetName(
        BasePackageName,
        TEXT("_"),
        UniquePackageName,
        UniqueAssetName
    );

    // ユニーク化されたパッケージからフォルダを取得
    const FString UniquePackagePath =
        FPackageName::GetLongPackagePath(
            UniquePackageName
        );


    // ==========================================
    // Material Instance用Factoryを生成
    // ==========================================

    UMaterialInstanceConstantFactoryNew* Factory =
        NewObject<UMaterialInstanceConstantFactoryNew>();

    // 親Materialを指定
    Factory->InitialParent = ParentMaterial;


    // ==========================================
    // Material Instanceを作成
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
            TEXT("Material Instanceの生成に失敗しました。保存先フォルダが存在するか確認してください。");

        return nullptr;
    }


    // ==========================================
    // 変更を反映
    // ==========================================

    CreatedInstance->PostEditChange();
    CreatedInstance->MarkPackageDirty();

    return CreatedInstance;
}
