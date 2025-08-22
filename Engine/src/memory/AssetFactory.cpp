#include "AssetFactory.h"

std::map<AssetType, AssetFactory::LoadFn> AssetFactory::loadFunctionsRegistry;

bool AssetFactory::loadAsset(AssetInfo aInfo)
{
	auto iter = loadFunctionsRegistry.find(aInfo.aType);
	if (iter == loadFunctionsRegistry.end())
	{
		logError("Invalid asset type in regsitry, did you forget to register your load function?");
		return false;
	}
	iter->second(aInfo);

	return true;
}

void AssetFactory::registerLoadFunc(AssetType aType, const LoadFn& fn)
{
	loadFunctionsRegistry[aType] = fn;
}
