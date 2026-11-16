#include "Core/ToonFacePreviewTextureImporter.h"

#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetImportTask.h"

#include "Engine/Texture2D.h"
#include "Engine/TextureDefines.h"

#include "HAL/FileManager.h"
#include "Modules/ModuleManager.h"

#include "Misc/Paths.h"
#include "Misc/PackageName.h"

#include "UObject/Package.h"
#include "UObject/SavePackage.h"


// ==================================================
// Import Face Normal Preview
// ==================================================

bool FToonFacePreviewTextureImporter::ImportNormalPreview(
    const FString& SourcePngPath,
    FString& OutObjectPath,
    FString& OutErrorMessage
)
{
    OutObjectPath.Reset();
    OutErrorMessage.Reset();


    // ==========================================
    // 1. Source Validation
    // ==========================================

    if (!FPaths::FileExists(SourcePngPath))
    {
        OutErrorMessage = FString::Printf(
            TEXT("PNGファイルが存在しません。\n%s"),
            *SourcePngPath
        );

        return false;
    }


    // ==========================================
    // 2. Import Task
    // ==========================================

    UAssetImportTask* ImportTask =
        NewObject<UAssetImportTask>(GetTransientPackage());

    if (!ImportTask)
    {
        OutErrorMessage =
            TEXT("Asset Import Taskを生成できません。");

        return false;
    }

    ImportTask->Filename = SourcePngPath;

    ImportTask->DestinationPath =
        TEXT("/Game/ToonGenerated/FaceMaps");

    ImportTask->bAutomated = true;

    ImportTask->bAsync = false;

    ImportTask->bSave = false;

    // 同じ名前の生成済みAssetを更新する
    ImportTask->bReplaceExisting = true;

    ImportTask->bReplaceExistingSettings = true;


    // ==========================================
    // 3. Execute Import
    // ==========================================

    FAssetToolsModule& AssetToolsModule =
        FModuleManager::LoadModuleChecked<FAssetToolsModule>(
            TEXT("AssetTools")
        );

    TArray<UAssetImportTask*> ImportTasks;

    ImportTasks.Add(ImportTask);

    AssetToolsModule.Get().ImportAssetTasks(ImportTasks);


    // ==========================================
    // 4. Get Imported Texture
    // ==========================================

    UTexture2D* ImportedTexture = nullptr;

    for (UObject* ImportedObject : ImportTask->GetObjects())
    {
        ImportedTexture = Cast<UTexture2D>(ImportedObject);

        if (IsValid(ImportedTexture))
        {
            break;
        }
    }

    if (!IsValid(ImportedTexture))
    {
        OutErrorMessage =
            TEXT("PNGからTexture2Dを取得できませんでした。");

        return false;
    }


    // ==========================================
    // 5. Texture Settings
    // ==========================================

    // 法線方向データはLinear Spaceで扱う
    ImportedTexture->SRGB = false;

    // RGBの方向情報を保持する形式
    ImportedTexture->CompressionSettings =
        TC_VectorDisplacementmap;

    // Textureの変更を反映
    ImportedTexture->PostEditChange();

    ImportedTexture->MarkPackageDirty();


    // ==========================================
    // 6. Save Package
    // ==========================================

    UPackage* Package =
        ImportedTexture->GetOutermost();

    if (!IsValid(Package))
    {
        OutErrorMessage =
            TEXT("Texture Packageを取得できません。");

        return false;
    }

    const FString PackageFilename =
        FPackageName::LongPackageNameToFilename(
            Package->GetName(),
            FPackageName::GetAssetPackageExtension()
        );

    FSavePackageArgs SaveArgs;

    SaveArgs.TopLevelFlags =
        RF_Public | RF_Standalone;

    SaveArgs.SaveFlags = SAVE_None;

    const bool bSaved = UPackage::SavePackage(
        Package,
        ImportedTexture,
        *PackageFilename,
        SaveArgs
    );

    if (!bSaved)
    {
        OutErrorMessage =
            TEXT("Texture Assetの保存に失敗しました。");

        return false;
    }


    // ==========================================
    // 7. Result
    // ==========================================

    OutObjectPath = ImportedTexture->GetPathName();

    return true;
}