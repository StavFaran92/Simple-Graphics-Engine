#include "memory/ResourceWrapper.h"
#include "memory/AssetRecord.h"
#include "core/Engine.h"
#include "memory/ResourceManager.h"
#include "memory/AssetFactory.h"
#include "AssetHandle.h"

#include "memory/AssetHandle.h"

ResourceWrapper<Resource> Test::loadAssetResourceInternal(const AssetRecord& record, UUID uuid)
{
    ResourceLoadDescriptor* loadDesc =
        AssetFactory::getManager(record.aType)->makeResourceLoadDescriptor();

    assert(loadDesc);

    ResourceWrapper<Resource> resource = loadDesc->loadResource();

    Engine::get()->getSubSystem<Assets>()->bindResourceToAsset(uuid, resource.getUID());

    return resource;
}
