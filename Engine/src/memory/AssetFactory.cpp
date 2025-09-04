#include "AssetFactory.h"

//std::map<AssetType, AssetFactory::LoadFn> AssetFactory::loadFunctionsRegistry;

bool AssetFactory::loadAsset(AssetInfo& aInfo)
{
	auto iter = getLoadFunctionRegistry().find(aInfo.aType);
	if (iter == getLoadFunctionRegistry().end())
	{
		logError("Invalid asset type in regsitry, did you forget to register your load function?");
		return false;
	}
	aInfo.data = iter->second(aInfo);

	return true;
}

void AssetFactory::registerLoadFunc(AssetType aType, const LoadFn& fn)
{
	getLoadFunctionRegistry()[aType] = fn;
}
