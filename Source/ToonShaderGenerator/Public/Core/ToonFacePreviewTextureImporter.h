#pragma once

#include "CoreMinimal.h"

class FToonFacePreviewTextureImporter
{
public:
	
	static bool ImportNormalPreview(
		const FString& SourcePngPath,
		FString& OutObjectPath,
		FString& OutErrorMessage
	);

	// FaceSDF(R=右光 / G=左光)のPNGをTextureとしてインポートする
	static bool ImportFaceSDF(
		const FString& SourcePngPath,
		FString& OutObjectPath,
		FString& OutErrorMessage
	);
};