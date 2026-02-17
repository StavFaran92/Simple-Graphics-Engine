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
	return false;
}

ResourceLoadDescriptor* PrefabTypeManager::makeResourceLoadDescriptor()
{
	return nullptr;
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

