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

        // UEのMeshDescriptionのUVは、すでにテクスチャ座標(左上原点・V下向き)なので
        // PNGの行(上から下)とそのまま対応する。Vは反転しない。
        const float Y =
            UV.Y
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
    FString& OutErrorMessage,
    float NormalSmoothingStrength,
    const FToonFaceSDFSettings& SDFSettings
)
{
    const float SmoothingStrength =
        FMath::Clamp(NormalSmoothingStrength, 0.0f, 1.0f);

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

    const auto VertexPositions =
        Attributes.GetVertexPositions();


    // ==========================================
    // Smooth Normal Center
    // ==========================================
    //
    // 顔の頂点法線は細かくバラつくため、そのまま使うと
    // 影の境界がザラザラになる。
    // 「各頂点の法線直線に最も近い点」を球状の中心として推定し、
    // 中心から外向きの方向へ法線を寄せる。

    FVector3f SmoothCenter = FVector3f::ZeroVector;
    float RadialSign = 1.0f;

    // 顔の前方向の自動検出用(法線の平均)
    FVector3f AverageNormal = FVector3f::ZeroVector;
    int32 AverageNormalCount = 0;

    if (SmoothingStrength > 0.0f
        || (SDFSettings.bGenerate && SDFSettings.bAutoDetectAxes))
    {
        FBox3f PositionBounds(ForceInit);

        TArray<FVector3f> SamplePositions;
        TArray<FVector3f> SampleNormals;

        for (const FPolygonGroupID PolygonGroupID :
            MeshDescription->PolygonGroups().GetElementIDs())
        {
            if (!CandidateSlotNames.Contains(
                PolygonGroupMaterialSlotNames[PolygonGroupID]))
            {
                continue;
            }

            for (const FTriangleID TriangleID :
                MeshDescription->GetPolygonGroupTriangles(
                    PolygonGroupID))
            {
                for (const FVertexInstanceID VI :
                    MeshDescription->GetTriangleVertexInstances(
                        TriangleID))
                {
                    FVector3f Normal = VertexInstanceNormals[VI];

                    if (!Normal.Normalize())
                    {
                        continue;
                    }

                    const FVector3f Position =
                        VertexPositions[
                            MeshDescription->GetVertexInstanceVertex(VI)
                        ];

                    PositionBounds += Position;
                    SamplePositions.Add(Position);
                    SampleNormals.Add(Normal);

                    AverageNormal += Normal;
                    ++AverageNormalCount;
                }
            }
        }

        if (SamplePositions.Num() > 0)
        {
            const FVector3f BoundsCenter =
                PositionBounds.GetCenter();

            const double BoundsDiagonal =
                PositionBounds.GetSize().Size();

            SmoothCenter = BoundsCenter;

            // 最小二乗: Σ(I - n nT) c = Σ(I - n nT) p
            double A[3][3] = {};
            double B[3] = {};

            for (int32 i = 0; i < SamplePositions.Num(); ++i)
            {
                const FVector3f& N = SampleNormals[i];
                const FVector3f& P = SamplePositions[i];

                const double n[3] = { N.X, N.Y, N.Z };
                const double p[3] = { P.X, P.Y, P.Z };

                for (int32 r = 0; r < 3; ++r)
                {
                    for (int32 c = 0; c < 3; ++c)
                    {
                        const double M =
                            (r == c ? 1.0 : 0.0) - n[r] * n[c];

                        A[r][c] += M;
                        B[r] += M * p[c];
                    }
                }
            }

            auto Det3 = [](const double M[3][3])
            {
                return
                    M[0][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1])
                    - M[0][1] * (M[1][0] * M[2][2] - M[1][2] * M[2][0])
                    + M[0][2] * (M[1][0] * M[2][1] - M[1][1] * M[2][0]);
            };

            const double Count =
                static_cast<double>(SamplePositions.Num());

            const double DetA = Det3(A);

            // 法線がほぼ平行(平面に近い)なら解が不安定なのでBounds中心を使う
            if (FMath::Abs(DetA) > 1.0e-4 * Count * Count * Count)
            {
                double Solution[3] = {};

                for (int32 k = 0; k < 3; ++k)
                {
                    double Ak[3][3];

                    for (int32 r = 0; r < 3; ++r)
                    {
                        for (int32 c = 0; c < 3; ++c)
                        {
                            Ak[r][c] = (c == k) ? B[r] : A[r][c];
                        }
                    }

                    Solution[k] = Det3(Ak) / DetA;
                }

                const FVector3f Estimated(
                    static_cast<float>(Solution[0]),
                    static_cast<float>(Solution[1]),
                    static_cast<float>(Solution[2])
                );

                // 極端に遠い場合は不採用
                if ((Estimated - BoundsCenter).Size()
                    < BoundsDiagonal * 20.0)
                {
                    SmoothCenter = Estimated;
                }
            }

            // 中心から外向きが元の法線と逆なら符号を反転
            double DotSum = 0.0;

            for (int32 i = 0; i < SamplePositions.Num(); ++i)
            {
                FVector3f Radial =
                    SamplePositions[i] - SmoothCenter;

                if (Radial.Normalize())
                {
                    DotSum += FVector3f::DotProduct(
                        Radial,
                        SampleNormals[i]
                    );
                }
            }

            RadialSign = (DotSum >= 0.0) ? 1.0f : -1.0f;
        }
    }

    auto SmoothNormal =
        [&](const FVertexInstanceID VI,
            const FVector3f& Normal) -> FVector3f
    {
        if (SmoothingStrength <= 0.0f)
        {
            return Normal;
        }

        FVector3f Radial =
            (VertexPositions[
                MeshDescription->GetVertexInstanceVertex(VI)
            ] - SmoothCenter) * RadialSign;

        if (!Radial.Normalize())
        {
            return Normal;
        }

        FVector3f Result =
            FMath::Lerp(Normal, Radial, SmoothingStrength);

        return Result.Normalize() ? Result : Normal;
    };


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

            // 頂点法線のバラつきを抑えて滑らかな法線にする
            N0 = SmoothNormal(VI0, N0);
            N1 = SmoothNormal(VI1, N1);
            N2 = SmoothNormal(VI2, N2);


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

    TArray<FVector3f> FinalNormals;
    FinalNormals.Init(
        FVector3f::ZeroVector,
        PixelCount
    );

    TArray<uint8> Filled;
    Filled.Init(0, PixelCount);


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

        FinalNormals[PixelIndex] = FinalNormal;
        Filled[PixelIndex] = 1;

        CoveragePixels[PixelIndex] =
            FColor::White;

        ++OutResult.RasterizedPixelCount;
    }


    // ==========================================
    // Padding (UVアイランド外へ法線を広げる)
    // ==========================================
    //
    // バイリニア補間でアイランド境界に黒い筋や
    // 別アイランドの値が混ざるのを防ぐ。

    const int32 PaddingPixels =
        FMath::Max(4, Resolution / 256);

    for (int32 Pass = 0; Pass < PaddingPixels; ++Pass)
    {
        TArray<FVector3f> NextNormals = FinalNormals;
        TArray<uint8> NextFilled = Filled;

        bool bChanged = false;

        for (int32 Y = 0; Y < Resolution; ++Y)
        {
            for (int32 X = 0; X < Resolution; ++X)
            {
                const int32 Index = Y * Resolution + X;

                if (Filled[Index])
                {
                    continue;
                }

                FVector3f Sum = FVector3f::ZeroVector;
                int32 NeighborCount = 0;

                for (int32 DY = -1; DY <= 1; ++DY)
                {
                    for (int32 DX = -1; DX <= 1; ++DX)
                    {
                        if (DX == 0 && DY == 0)
                        {
                            continue;
                        }

                        const int32 NX = X + DX;
                        const int32 NY = Y + DY;

                        if (NX < 0 || NX >= Resolution
                            || NY < 0 || NY >= Resolution)
                        {
                            continue;
                        }

                        const int32 NeighborIndex =
                            NY * Resolution + NX;

                        if (Filled[NeighborIndex])
                        {
                            Sum += FinalNormals[NeighborIndex];
                            ++NeighborCount;
                        }
                    }
                }

                if (NeighborCount > 0 && Sum.Normalize())
                {
                    NextNormals[Index] = Sum;
                    NextFilled[Index] = 1;
                    bChanged = true;
                }
            }
        }

        FinalNormals = MoveTemp(NextNormals);
        Filled = MoveTemp(NextFilled);

        if (!bChanged)
        {
            break;
        }
    }

    for (int32 PixelIndex = 0;
        PixelIndex < PixelCount;
        ++PixelIndex)
    {
        if (Filled[PixelIndex])
        {
            NormalPixels[PixelIndex] =
                EncodeNormal(FinalNormals[PixelIndex]);
        }
    }


    // ==========================================
    // Face SDF
    // ==========================================
    //
    // ライトを水平に「正面(0) → 側面(π/2) → 背面(π)」へ回したとき、
    // 各ピクセルが明るい→影に変わる角度を 0〜1 (=0〜π) で保存する。
    //   R: 右から光が来る場合
    //   G: 左から光が来る場合 (右左反転した別計算なのでUV対称性は不要)
    //
    // マテリアル側は「光の角度 a(0=正面,1=背面) < SDF値」なら明るい。
    //
    // 法線の水平成分を (Nf=前, Nr=右) とすると、
    //   Dot = A * cos(φ - φ0)  (A = |(Nf,Nr)|, φ0 = atan2(Nr, Nf))
    // がしきい値Tを下回る角度φを解析的に求める(滑らかな法線ならSDFも滑らか)。

    TArray<FColor> SDFPixels;

    if (SDFSettings.bGenerate)
    {
        SDFPixels.Init(
            FColor(0, 0, 0, 255),
            PixelCount
        );

        FVector3f Forward =
            SDFSettings.ForwardAxis.GetSafeNormal();

        FVector3f Right =
            SDFSettings.RightAxis.GetSafeNormal();

        // 顔の面は前方を向いているので、頂点法線の平均の水平成分が
        // 顔の前方向になる(反対向きのキャラクターでも自動で追従する)
        if (SDFSettings.bAutoDetectAxes && AverageNormalCount > 0)
        {
            const FVector3f Up = SDFSettings.UpAxis.GetSafeNormal();

            const FVector3f MeanNormal =
                AverageNormal / static_cast<float>(AverageNormalCount);

            FVector3f Horizontal =
                MeanNormal
                - Up * FVector3f::DotProduct(MeanNormal, Up);

            // 平均が小さすぎる(顔以外も含んで打ち消し合っている)場合は不採用
            if (Horizontal.Size() > 0.1f && Horizontal.Normalize())
            {
                Forward = Horizontal;
                Right = FVector3f::CrossProduct(Up, Forward).GetSafeNormal();
            }
        }

        OutResult.SDFForwardAxis = Forward;
        OutResult.SDFRightAxis = Right;

        const float Threshold =
            FMath::Clamp(SDFSettings.ShadowThreshold, -0.95f, 0.95f);

        auto CalculateThresholdAngle =
            [Threshold](float Nf, float Nr) -> float
        {
            const float A = FMath::Sqrt(Nf * Nf + Nr * Nr);

            // 水平成分がほぼ無い(真上/真下向き)場合は影扱い
            if (A < KINDA_SMALL_NUMBER)
            {
                return 0.0f;
            }

            const float Ratio = Threshold / A;

            // 常に明るい
            if (Ratio <= -1.0f)
            {
                return 1.0f;
            }

            // 常に影
            if (Ratio >= 1.0f)
            {
                return 0.0f;
            }

            // 正面(φ=0)の時点で既に影なら、この回り方では点灯しない
            if (Nf <= Threshold)
            {
                return 0.0f;
            }

            const float Phi0 = FMath::Atan2(Nr, Nf);
            const float PhiEnd = Phi0 + FMath::Acos(Ratio);

            return FMath::Clamp(PhiEnd / PI, 0.0f, 1.0f);
        };

        for (int32 PixelIndex = 0;
            PixelIndex < PixelCount;
            ++PixelIndex)
        {
            if (!Filled[PixelIndex])
            {
                continue;
            }

            const FVector3f& N = FinalNormals[PixelIndex];

            const float Nf = FVector3f::DotProduct(N, Forward);
            const float Nr = FVector3f::DotProduct(N, Right);

            const float RightLight =
                CalculateThresholdAngle(Nf, Nr);

            const float LeftLight =
                CalculateThresholdAngle(Nf, -Nr);

            SDFPixels[PixelIndex] = FColor(
                static_cast<uint8>(
                    FMath::Clamp(
                        FMath::RoundToInt(RightLight * 255.0f),
                        0, 255)),
                static_cast<uint8>(
                    FMath::Clamp(
                        FMath::RoundToInt(LeftLight * 255.0f),
                        0, 255)),
                0,
                255
            );
        }
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

    bool bSDFSaved = true;

    if (SDFSettings.bGenerate)
    {
        OutResult.SDFPreviewPath =
            FPaths::Combine(
                OutputDirectory,
                FString::Printf(
                    TEXT("FaceSDF_Slot_%d.png"),
                    MaterialSlotIndex
                )
            );

        bSDFSaved = SavePNG(
            OutResult.SDFPreviewPath,
            Resolution,
            Resolution,
            SDFPixels
        );
    }

    if (!bNormalSaved || !bCoverageSaved || !bSDFSaved)
    {
        OutErrorMessage =
            TEXT("プレビュー画像の保存に失敗しました。");

        return false;
    }


    OutResult.Resolution = Resolution;

    return true;
}