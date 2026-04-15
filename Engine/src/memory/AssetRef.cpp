#include "AssetRef.h"

#include "memory/ResourceRef.h"
#include "memory/AssetRecord.h"
#include "core/Engine.h"
#include "memory/ResourceManager.h"
#include "memory/AssetFactory.h"
#include "memory/Asset.h"

ResourceRef<Resource> AssetHandleImpl::loadAssetResourceInternal(const AssetRecord& record, UUID uuid)
{
    std::unique_ptr< ResourceLoadDescriptor> loadDesc = AssetFactory::getManager(record.aType)->makeResourceLoadDescriptor();

    assert(loadDesc);

    record.asset->fillLoadDescriptor(*loadDesc);

    loadDesc->sourcePath = record.getAbsolutePath();

    ResourceRef<Resource> resource = AssetFactory::getManager(record.aType)->loadResourceFromDisk(*loadDesc);

    if (resource.isEmpty())
    {
        logError("Failed to load resource: {}", uuid.str());
        return ResourceRef<Resource>::empty;
    }

    record.asset->fillData(resource);

    Engine::get()->getSubSystem<Assets>()->bindResourceToAsset(uuid, resource.getUID());

    return resource;
}

const AssetRecord& AssetHandleImpl::getInfo(UUID uuid)
{
    return Engine::get()->getSubSystem<Assets>()->getInfo(uuid);
}

ResourceRef<Resource> AssetHandleImpl::createOrGetCachedResource(const AssetRecord& record, UUID uuid)
{
    return Engine::get()->getResourceManager()->createOrGetCached(
        record.resourceID,
        [&record, uuid]() {
            return AssetHandleImpl::loadAssetResourceInternal(record, uuid);
        });
}

void AssetHandleImpl::syncAsset(UUID uuid)
{
    Engine::get()->getSubSystem<Assets>()->sync(uuid);
}

void AssetHandleImpl::deleteAsset(UUID uuid)
{
    Engine::get()->getSubSystem<Assets>()->deleteAsset(uuid);
}

void AssetHandleImpl::makeAssetDirty(UUID uuid)
{
    Engine::get()->getSubSystem<Assets>()->makeDirty(uuid);
}
