#include "AssetFactory.h"

#include "memory/Asset.h"
#include "core/Logger.h"

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