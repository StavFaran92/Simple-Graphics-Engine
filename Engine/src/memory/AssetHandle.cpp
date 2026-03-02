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

    loadDesc->sourcePath = Engine::get()->getProjectDirectory() + record.relativefilePath;

    ResourceWrapper<Resource> resource = loadDesc->loadResource();

    if (resource.isEmpty())
    {
        logError("Failed to load resource: {}", uuid.str());
        return ResourceWrapper<Resource>::empty;
    }

    record.asset->fillData(resource);

    Engine::get()->getSubSystem<Assets>()->bindResourceToAsset(uuid, resource.getUID());

    return resource;
}
