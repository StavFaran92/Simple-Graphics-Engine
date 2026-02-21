#include "PrefabManager.h"

#include "runtime/Prefab.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"

#include <fstream>
#include <cereal/archives/json.hpp>

Asset* PrefabTypeManager::createAsset(AssetCreateDescriptor& desc)
{
	return new PrefabAsset(desc);
}

bool PrefabTypeManager::importAsset(const std::string& src, const ScopedPath& dst)
{
	return false;
}

bool PrefabTypeManager::saveResource(const ResourceCreateDescriptor& desc, const ScopedPath& dst)
{
	auto prefabDesc = dynamic_cast<const PrefabCreateDescriptor*>(&desc);
	if (!prefabDesc)
	{
		logError("Invalid Descriptor specified.");
		return false;
	}

	std::ofstream os(dst.absolute());
	cereal::JSONOutputArchive oarchive(os);

	try
	{
		oarchive(prefabDesc->data);
	}
	catch (const cereal::Exception& e)
	{
		logError("Serialization Error occured: {}", e.what());
	}

	return true;
}

ResourceLoadDescriptor* PrefabTypeManager::makeResourceLoadDescriptor()
{
	return new PrefabLoadDescriptor();
}

ResourceWrapper<Resource> PrefabTypeManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	return ResourceWrapper<Resource>();
}

void PrefabTypeManager::parse(ResourceLoadDescriptor& desc)
{
}

void PrefabTypeManager::parse(ResourceCreateDescriptor& desc)
{
}

