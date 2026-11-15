#include "Core/ToonMasterMaterialRegistry.h"


bool FToonMasterMaterialRegistry::Resolve(EToonMaterialType MaterialType, FSoftObjectPath& OutMaterialPath,
	FString& OutMaterialName, FString& OutErrorMessage)
{
	
	OutMaterialPath.Reset();
	OutMaterialName.Reset();
	OutErrorMessage.Reset();
	
	FString AssetPath;

	switch (MaterialType)
	{
	case EToonMaterialType::Face:
		
		AssetPath = 
			TEXT("/ToonShaderGenerator/Materials/Master/M_Toon_Face.M_Toon_Face");
		
		OutMaterialName = TEXT("M_Toon_Face");
		
		break;
		
	case EToonMaterialType::Hair:
		
		AssetPath = 
			TEXT("/ToonShaderGenerator/Materials/Master/M_Toon_Hair.M_Toon_Hair");
		
		OutMaterialName = TEXT("M_Toon_Hair");
		
		break;
		
	case EToonMaterialType::Other:

		AssetPath =
			TEXT("/ToonShaderGenerator/Materials/Master/M_Toon_Basic.M_Toon_Basic");

		OutMaterialName = TEXT("M_Toon_Basic");

		break;


	case EToonMaterialType::Skin:

		OutErrorMessage =
			TEXT("Skin専用Master Materialはまだ実装されていません。");

		return false;


	case EToonMaterialType::Clothing:

		OutErrorMessage =
			TEXT("Clothing専用Master Materialはまだ実装されていません。");

		return false;	
	
	default:
		
		OutErrorMessage =
			TEXT("未対応のMaterial Typeです。");
		
		return false;
	}
	
	OutMaterialPath = FSoftObjectPath(AssetPath);
	
	return true;
}
