#pragma once

#include "CoreMinimal.h"

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
		FString& OutErrorMessage
	);
};
