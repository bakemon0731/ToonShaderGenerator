#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"

#include "Data/ToonMaterialSlotInfo.h"

class FToonMasterMaterialRegistry
{
public:
	
	// Material用途に応じてMaster Materialを解決する
	static bool Resolve(
		EToonMaterialType MaterialType,
		FSoftObjectPath& OutMaterialPath,
		FString& OutMaterialName,
		FString& OutErrorMessage
	);	
};