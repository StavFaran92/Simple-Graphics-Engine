#include "MaterialManager.h"

#include "render/Material.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"

#include <fstream>
#include <cereal/archives/json.hpp>

Asset* MaterialTypeManager::createAsset(AssetCreateDescriptor& desc)
{
	return new MaterialAsset(desc);
}

bool MaterialTypeManager::importAsset(const std::string& src, const ScopedPath& dst)
{
	return false;
}

bool MaterialTypeManager::saveResource(const ResourceCreateDescriptor& desc, const ScopedPath& dst)
{
	return false;
}

ResourceLoadDescriptor* MaterialTypeManager::makeResourceLoadDescriptor()
{
	return nullptr;
}

ResourceWrapper<Resource> MaterialTypeManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	return ResourceWrapper<Resource>();
}

void MaterialTypeManager::parse(ResourceLoadDescriptor& desc)
{
}

void MaterialTypeManager::parse(ResourceCreateDescriptor& desc)
{
}