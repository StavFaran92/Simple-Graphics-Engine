#include "AssetFactory.h"

#include "memory/Asset.h"
#include "core/Logger.h"

std::map<AssetType, std::shared_ptr<ResourceTypeManager>>& AssetFactory::getManagerRegistry()
{
    static std::map<AssetType, std::shared_ptr<ResourceTypeManager>> registry;
    return registry;
}

void AssetFactory::registerManager(AssetType aType, std::shared_ptr<ResourceTypeManager> manager)
{
    auto& registry = getManagerRegistry();
    registry[aType] = std::move(manager);
}

ResourceTypeManager* AssetFactory::getManager(AssetType aType)
{
    auto& registry = getManagerRegistry();
    auto it = registry.find(aType);
    if (it == registry.end()) {
        logError("ResourceTypeManager not registered for asset type {}", static_cast<int>(aType));
        return nullptr;
    }
    return it->second.get();
}

bool AssetFactory::loadAsset(AssetRecord& aInfo)
{
	auto iter = getLoadFunctionRegistry().find(aInfo.aType);
	if (iter == getLoadFunctionRegistry().end())
	{
		logError("Invalid asset type in regsitry, did you forget to register your load function?");
		return false;
	}

	return true;
}

void AssetFactory::registerLoadFunc(AssetType aType, const LoadFn& fn)
{
	getLoadFunctionRegistry()[aType] = fn;
}
