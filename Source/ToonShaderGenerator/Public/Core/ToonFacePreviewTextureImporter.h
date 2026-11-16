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
};