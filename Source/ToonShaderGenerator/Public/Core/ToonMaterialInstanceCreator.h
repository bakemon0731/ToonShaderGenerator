#pragma once

#include "CoreMinimal.h"
#include "Data/ToonMaterialInstanceSettings.h"

class UMaterialInterface;
class UMaterialInstanceConstant;

class FToonMaterialInstanceCreator
{
public:

	static UMaterialInstanceConstant* Create(
		UMaterialInterface* ParentMaterial,
		const FString& AssetName,
		const FString& PackagePath,
		const FToonMaterialInstanceSettings& Settings,
		bool& bOutWasUpdated,
		FString& OutErrorMessage
	);
};
