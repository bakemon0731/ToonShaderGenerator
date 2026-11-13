#pragma once

#include "CoreMinimal.h"
#include "Data/ToonBasicMaterialSettings.h"

class UMaterialInterface;
class UMaterialInstanceConstant;

class FToonMaterialInstanceCreator
{
public:

	// Material Instanceを生成する
	static UMaterialInstanceConstant* Create(
		UMaterialInterface* ParentMaterial,
		const FString& AssetName,
		const FString& PackagePath,
		const FToonBasicMaterialSettings& Settings,
		FString& OutErrorMessage
	);
};
