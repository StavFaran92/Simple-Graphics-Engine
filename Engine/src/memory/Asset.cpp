#include "Asset.h"

#include "core/Logger.h"
#include "memory/Assets.h"
#include "core/Engine.h"
#include "memory/ResourceManager.h"
#include "memory/AssetFactory.h"
#include "memory/FrameAccessTable.h"

#include <sstream>
#include <cereal/archives/json.hpp>
//
//Asset::Asset(const AssetBuildDescriptor& desc)
//	:m_createDesc (desc)
//{
//}

void Asset::syncAsset() const
{
    Engine::get()->getSubSystem<Assets>()->sync(uuid);
}

void Asset::deleteAsset()
{
    Engine::get()->getSubSystem<Assets>()->deleteAsset(uuid);
}

void Asset::makeAssetDirty()
{
    Engine::get()->getSubSystem<Assets>()->makeDirty(uuid);
}

const AssetRecord& Asset::info() const
{
    return Engine::get()->getSubSystem<Assets>()->getInfo(uuid);
}

ResourceRef<Resource> loadAssetResourceInternal(const AssetRecord& record, UUID uuid)
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

ResourceRef<Resource> Asset::resource() const
{
	if (isEmpty())
		return ResourceRef<Resource>::empty;

	const AssetRecord& record = info();

	auto resource = Engine::get()->getResourceManager()->createOrGetCached(
		record.resourceID,
		[&record, this]() {
			return loadAssetResourceInternal(record, uuid);
		});

    if (resource.isEmpty())
    {
        return ResourceRef<Resource>::empty;
    }

    Engine::get()->getSubSystem<FrameAccessTable>()->onResourceAccessed(resource);

	if (record.isResourceDirty())
	{
		fillData(resource);
		syncAsset();
	}

	return resource;
}

UUID Asset::getUID() const
{
    return uuid;
}

std::string Asset::saveToJson() const
{
    std::stringstream ss;
    {
        cereal::JSONOutputArchive archive(ss);
        save(archive);
    }

    // cereal writes nothing at all for an archive with no fields
    std::string json = ss.str();
    return json.empty() ? "{}" : json;
}

void Asset::loadFromJson(const std::string& json)
{
    std::stringstream ss(json);
    cereal::JSONInputArchive archive(ss);
    load(archive);
}
