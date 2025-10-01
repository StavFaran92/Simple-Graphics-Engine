#include "AssetFactory.h"

#include "memory/Asset.h"

//std::map<AssetType, AssetFactory::LoadFn> AssetFactory::loadFunctionsRegistry;

std::map<AssetType, std::shared_ptr<AssetManager>>& AssetFactory::getManagerRegistry()
{
    static std::map<AssetType, std::shared_ptr<AssetManager>> registry;
    return registry;
}

void AssetFactory::registerManager(AssetType aType, std::shared_ptr<AssetManager> manager)
{
    auto& registry = getManagerRegistry();
    registry[aType] = std::move(manager);
}

AssetManager* AssetFactory::getManager(AssetType aType)
{
    auto& registry = getManagerRegistry();
    auto it = registry.find(aType);
    if (it == registry.end()) {
        logError("AssetManager not registered for asset type {}", static_cast<int>(aType));
        return nullptr;
    }
    return it->second.get();
}

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
