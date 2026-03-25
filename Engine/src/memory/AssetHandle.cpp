#include "memory/ResourceWrapper.h"
#include "memory/AssetRecord.h"
#include "core/Engine.h"
#include "memory/ResourceManager.h"
#include "memory/AssetFactory.h"
#include "AssetHandle.h"

#include "memory/AssetHandle.h"

ResourceWrapper<Resource> Test::loadAssetResourceInternal(const AssetRecord& record, UUID uuid)
{
    ResourceLoadDescriptor* loadDesc = AssetFactory::getManager(record.aType)->makeResourceLoadDescriptor();

    assert(loadDesc);

    ScopedPath p = record.isEngineOwned ? ScopedPath::EnginePath() : ScopedPath::ContentPath();
    p.setPath(record.relativefilePath);
    loadDesc->sourcePath = p.absolute().string();

    ResourceWrapper<Resource> resource = AssetFactory::getManager(record.aType)->loadResourceFromDisk(*loadDesc);

    if (resource.isEmpty())
    {
        logError("Failed to load resource: {}", uuid.str());
        return ResourceWrapper<Resource>::empty;
    }

    record.asset->fillData(resource);

    Engine::get()->getSubSystem<Assets>()->bindResourceToAsset(uuid, resource.getUID());

    return resource;
}
