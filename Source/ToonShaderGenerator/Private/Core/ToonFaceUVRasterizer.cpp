#include "Core/ToonFaceUVRasterizer.h"

#include "Engine/SkeletalMesh.h"
#include "Engine/SkinnedAssetCommon.h"

#include "MeshDescription.h"
#include "SkeletalMeshAttributes.h"

#include "ImageUtils.h"

#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"


namespace
{
    // ==========================================
    // UV -> Image Pixel
    // ==========================================

    FVector2f UVToPixel(
        const FVector2f& UV,
        int32 Resolution
    )
    {
        const float X =
            UV.X * static_cast<float>(Resolution - 1);

        // PNGは上から下へ行が並ぶためVを反転
        const float Y =
            (1.0f - UV.Y)
            * static_cast<float>(Resolution - 1);

        return FVector2f(X, Y);
    }


    // ==========================================
    // Normal Encode
    // ==========================================

    uint8 EncodeNormalComponent(float Value)
    {
        const float Encoded =
            Value * 0.5f + 0.5f;

        const int32 ByteValue =
            FMath::RoundToInt(Encoded * 255.0f);

        return static_cast<uint8>(
            FMath::Clamp(ByteValue, 0, 255)
        );
    }


    FColor EncodeNormal(const FVector3f& Normal)
    {
        return FColor(
            EncodeNormalComponent(Normal.X),
            EncodeNormalComponent(Normal.Y),
            EncodeNormalComponent(Normal.Z),
            255
        );
    }


    // ==========================================
    // Barycentric Coordinates
    // ==========================================

    bool CalculateBarycentric(
        const FVector2f& Point,
        const FVector2f& A,
        const FVector2f& B,
        const FVector2f& C,
        float& OutWeightA,
        float& OutWeightB,
        float& OutWeightC
    )
    {
        const float Denominator =
            (B.Y - C.Y) * (A.X - C.X)
            + (C.X - B.X) * (A.Y - C.Y);

        if (FMath::Abs(Denominator) < 0.000001f)
        {
            return false;
        }

        OutWeightA =
            (
                (B.Y - C.Y) * (Point.X - C.X)
                + (C.X - B.X) * (Point.Y - C.Y)
            ) / Denominator;

        OutWeightB =
            (
                (C.Y - A.Y) * (Point.X - C.X)
                + (A.X - C.X) * (Point.Y - C.Y)
            ) / Denominator;

        OutWeightC =
            1.0f - OutWeightA - OutWeightB;

        constexpr float Epsilon = 0.0001f;

        return
            OutWeightA >= -Epsilon
            && OutWeightB >= -Epsilon
            && OutWeightC >= -Epsilon;
    }


    // ==========================================
    // PNG Save
    // ==========================================

    bool SavePNG(
        const FString& FilePath,
        int32 Width,
        int32 Height,
        const TArray<FColor>& Pixels
    )
    {
        TArray<uint8> CompressedData;

        FImageUtils::CompressImageArray(
            Width,
            Height,
            Pixels,
            CompressedData
        );

        return FFileHelper::SaveArrayToFile(
            CompressedData,
            *FilePath
        );
    }
}


// ==================================================
// Generate Normal Preview
// ==================================================

bool FToonFaceUVRasterizer::GenerateNormalPreview(
    const USkeletalMesh* SkeletalMesh,
    int32 MaterialSlotIndex,
    int32 LODIndex,
    int32 Resolution,
    FToonFaceUVRasterResult& OutResult,
    FString& OutErrorMessage
)
{
    OutResult = FToonFaceUVRasterResult();
    OutErrorMessage.Reset();


    // ==========================================
    // Input Validation
    // ==========================================

    if (!IsValid(SkeletalMesh))
    {
        OutErrorMessage =
            TEXT("Skeletal Meshが無効です。");

        return false;
    }

    if (Resolution < 64 || Resolution > 4096)
    {
        OutErrorMessage =
            TEXT("解像度は64～4096の範囲にしてください。");

        return false;
    }

    const TArray<FSkeletalMaterial>& Materials =
        SkeletalMesh->GetMaterials();

    if (!Materials.IsValidIndex(MaterialSlotIndex))
    {
        OutErrorMessage =
            TEXT("Material Slot Indexが無効です。");

        return false;
    }


    // ==========================================
    // MeshDescription
    // ==========================================

    const FMeshDescription* MeshDescription =
        SkeletalMesh->GetMeshDescription(LODIndex);

    if (!MeshDescription)
    {
        OutErrorMessage =
            TEXT("MeshDescriptionを取得できません。");

        return false;
    }

    if (MeshDescription->GetNumUVElementChannels() <= 0)
    {
        OutErrorMessage =
            TEXT("UV Channelがありません。");

        return false;
    }


    // ==========================================
    // Material Slot Names
    // ==========================================

    const FSkeletalMaterial& SkeletalMaterial =
        Materials[MaterialSlotIndex];

    TArray<FName> CandidateSlotNames;

    if (!SkeletalMaterial.MaterialSlotName.IsNone())
    {
        CandidateSlotNames.AddUnique(
            SkeletalMaterial.MaterialSlotName
        );
    }

    if (!SkeletalMaterial.ImportedMaterialSlotName.IsNone())
    {
        CandidateSlotNames.AddUnique(
            SkeletalMaterial.ImportedMaterialSlotName
        );
    }

    if (CandidateSlotNames.IsEmpty())
    {
        OutErrorMessage =
            TEXT("Material Slot名を取得できません。");

        return false;
    }


    // ==========================================
    // Mesh Attributes
    // ==========================================

    FSkeletalMeshAttributes Attributes(
        *const_cast<FMeshDescription*>(MeshDescription)
    );

    const auto PolygonGroupMaterialSlotNames =
        Attributes.GetPolygonGroupMaterialSlotNames();

    const auto VertexInstanceUVs =
        Attributes.GetVertexInstanceUVs();

    const auto VertexInstanceNormals =
        Attributes.GetVertexInstanceNormals();


    // ==========================================
    // Pixel Buffers
    // ==========================================

    const int32 PixelCount =
        Resolution * Resolution;

    TArray<FVector3f> NormalAccumulation;
    NormalAccumulation.Init(
        FVector3f::ZeroVector,
        PixelCount
    );

    TArray<FVector3f> FirstNormal;
    FirstNormal.Init(
        FVector3f::ZeroVector,
        PixelCount
    );

    TArray<uint32> HitCounts;
    HitCounts.Init(0, PixelCount);


    // ==========================================
    // Material Slot Rasterization
    // ==========================================

    bool bFoundMaterialGroup = false;

    for (const FPolygonGroupID PolygonGroupID :
        MeshDescription->PolygonGroups().GetElementIDs())
    {
        const FName GroupSlotName =
            PolygonGroupMaterialSlotNames[PolygonGroupID];

        if (!CandidateSlotNames.Contains(GroupSlotName))
        {
            continue;
        }

        bFoundMaterialGroup = true;

        const auto Triangles =
            MeshDescription->GetPolygonGroupTriangles(
                PolygonGroupID
            );

        for (const FTriangleID TriangleID : Triangles)
        {
            const auto VertexInstances =
                MeshDescription->GetTriangleVertexInstances(
                    TriangleID
                );

            if (VertexInstances.Num() != 3)
            {
                continue;
            }

            const FVertexInstanceID VI0 = VertexInstances[0];
            const FVertexInstanceID VI1 = VertexInstances[1];
            const FVertexInstanceID VI2 = VertexInstances[2];


            // ----------------------------------
            // UV取得
            // ----------------------------------

            const FVector2f UV0 =
                VertexInstanceUVs.Get(VI0, 0);

            const FVector2f UV1 =
                VertexInstanceUVs.Get(VI1, 0);

            const FVector2f UV2 =
                VertexInstanceUVs.Get(VI2, 0);


            // ----------------------------------
            // Normal取得
            // ----------------------------------

            FVector3f N0 =
                VertexInstanceNormals[VI0];

            FVector3f N1 =
                VertexInstanceNormals[VI1];

            FVector3f N2 =
                VertexInstanceNormals[VI2];

            if (!N0.Normalize()
                || !N1.Normalize()
                || !N2.Normalize())
            {
                continue;
            }


            // ----------------------------------
            // UV -> Pixel
            // ----------------------------------

            const FVector2f P0 =
                UVToPixel(UV0, Resolution);

            const FVector2f P1 =
                UVToPixel(UV1, Resolution);

            const FVector2f P2 =
                UVToPixel(UV2, Resolution);


            // ----------------------------------
            // Bounding Box
            // ----------------------------------

            const float MinFloatX =
                FMath::Min3(P0.X, P1.X, P2.X);

            const float MaxFloatX =
                FMath::Max3(P0.X, P1.X, P2.X);

            const float MinFloatY =
                FMath::Min3(P0.Y, P1.Y, P2.Y);

            const float MaxFloatY =
                FMath::Max3(P0.Y, P1.Y, P2.Y);

            const int32 MinX = FMath::Clamp(
                FMath::FloorToInt(MinFloatX),
                0,
                Resolution - 1
            );

            const int32 MaxX = FMath::Clamp(
                FMath::CeilToInt(MaxFloatX),
                0,
                Resolution - 1
            );

            const int32 MinY = FMath::Clamp(
                FMath::FloorToInt(MinFloatY),
                0,
                Resolution - 1
            );

            const int32 MaxY = FMath::Clamp(
                FMath::CeilToInt(MaxFloatY),
                0,
                Resolution - 1
            );


            // ----------------------------------
            // Triangle Rasterization
            // ----------------------------------

            bool bRasterizedTriangle = false;

            for (int32 Y = MinY; Y <= MaxY; ++Y)
            {
                for (int32 X = MinX; X <= MaxX; ++X)
                {
                    const FVector2f SamplePoint(
                        static_cast<float>(X),
                        static_cast<float>(Y)
                    );

                    float W0 = 0.0f;
                    float W1 = 0.0f;
                    float W2 = 0.0f;

                    if (!CalculateBarycentric(
                        SamplePoint,
                        P0,
                        P1,
                        P2,
                        W0,
                        W1,
                        W2))
                    {
                        continue;
                    }

                    // ----------------------------------
                    // Normal Interpolation
                    // ----------------------------------

                    FVector3f InterpolatedNormal =
                        N0 * W0
                        + N1 * W1
                        + N2 * W2;

                    if (!InterpolatedNormal.Normalize())
                    {
                        continue;
                    }

                    const int32 PixelIndex =
                        Y * Resolution + X;

                    if (HitCounts[PixelIndex] == 0)
                    {
                        FirstNormal[PixelIndex] =
                            InterpolatedNormal;
                    }
                    else if (HitCounts[PixelIndex] == 1)
                    {
                        ++OutResult.OverlappedPixelCount;
                    }

                    NormalAccumulation[PixelIndex] +=
                        InterpolatedNormal;

                    ++HitCounts[PixelIndex];

                    bRasterizedTriangle = true;
                }
            }

            if (bRasterizedTriangle)
            {
                ++OutResult.TriangleCount;
            }
        }
    }


    // ==========================================
    // Material Group Validation
    // ==========================================

    if (!bFoundMaterialGroup)
    {
        OutErrorMessage =
            TEXT("対象Material Slotに対応するPolygon Groupがありません。");

        return false;
    }

    if (OutResult.TriangleCount == 0)
    {
        OutErrorMessage =
            TEXT("ラスタライズできる三角形がありません。");

        return false;
    }


    // ==========================================
    // Create Output Images
    // ==========================================

    TArray<FColor> NormalPixels;
    NormalPixels.Init(
        FColor(0, 0, 0, 0),
        PixelCount
    );

    TArray<FColor> CoveragePixels;
    CoveragePixels.Init(
        FColor(0, 0, 0, 255),
        PixelCount
    );


    for (int32 PixelIndex = 0;
        PixelIndex < PixelCount;
        ++PixelIndex)
    {
        const uint32 HitCount =
            HitCounts[PixelIndex];

        if (HitCount == 0)
        {
            continue;
        }

        FVector3f FinalNormal =
            NormalAccumulation[PixelIndex]
            / static_cast<float>(HitCount);

        if (!FinalNormal.Normalize())
        {
            FinalNormal =
                FirstNormal[PixelIndex];
        }

        NormalPixels[PixelIndex] =
            EncodeNormal(FinalNormal);

        CoveragePixels[PixelIndex] =
            FColor::White;

        ++OutResult.RasterizedPixelCount;
    }


    // ==========================================
    // Save Preview PNG
    // ==========================================

    const FString OutputDirectory =
        FPaths::Combine(
            FPaths::ProjectSavedDir(),
            TEXT("ToonShaderGenerator")
        );

    IFileManager::Get().MakeDirectory(
        *OutputDirectory,
        true
    );

    OutResult.NormalPreviewPath =
        FPaths::Combine(
            OutputDirectory,
            FString::Printf(
                TEXT("FaceNormalPreview_Slot_%d.png"),
                MaterialSlotIndex
            )
        );

    OutResult.CoveragePreviewPath =
        FPaths::Combine(
            OutputDirectory,
            FString::Printf(
                TEXT("FaceCoveragePreview_Slot_%d.png"),
                MaterialSlotIndex
            )
        );

    const bool bNormalSaved = SavePNG(
        OutResult.NormalPreviewPath,
        Resolution,
        Resolution,
        NormalPixels
    );

    const bool bCoverageSaved = SavePNG(
        OutResult.CoveragePreviewPath,
        Resolution,
        Resolution,
        CoveragePixels
    );

    if (!bNormalSaved || !bCoverageSaved)
    {
        OutErrorMessage =
            TEXT("プレビュー画像の保存に失敗しました。");

        return false;
    }


    OutResult.Resolution = Resolution;

    return true;
}