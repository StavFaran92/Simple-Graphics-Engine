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

bool MaterialTypeManager::importAsset(const std::string& src, ImportNode& result)
{
	// Material import not implemented
	return false;
}

bool MaterialTypeManager::saveResource(const ResourceCreateDescriptor& desc, const ScopedPath& dst)
{
	auto materialDesc = dynamic_cast<const MaterialCreateDescriptor*>(&desc);
	if (!materialDesc)
	{
		logError("Invalid Descriptor specified.");
		return false;
	}

	std::ofstream os(dst.absolute());
	cereal::JSONOutputArchive oarchive(os);

	try
	{
		oarchive(materialDesc->data);
	}
	catch (const cereal::Exception& e)
	{
		logError("Serialization Error occured: {}", e.what());
	}

	return true;
}

ResourceLoadDescriptor* MaterialTypeManager::makeResourceLoadDescriptor()
{
	return new MaterialLoadDescriptor();
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