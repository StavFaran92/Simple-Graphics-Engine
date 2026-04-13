#include "MaterialManager.h"

#include "render/Material.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"
#include "render/MaterialDataParser.h"

#include <fstream>
#include <cereal/archives/json.hpp>

Ref<Asset> MaterialTypeManager::createAsset(const AssetBuildDescriptor& assetDesc, const ResourceBuildDescriptor& resourceDesc)
{
	auto materialDesc = dynamic_cast<const MaterialCreateDescriptor*>(&resourceDesc);
	if (!materialDesc)
	{
		logError("Invalid Descriptor specified.");
		return nullptr;
	}
	return createRef< MaterialAsset>(*materialDesc);
}

Ref<Asset> MaterialTypeManager::deserializeAsset(const nlohmann::json& j)
{
	auto asset = createRef<MaterialAsset>();
	asset->deserialize(j);
	return asset;
}

bool MaterialTypeManager::importAsset(const ResourceLoadDescriptor& loadDesc, ImportNode& result)
{
	// Material import not implemented
	return false;
}

bool MaterialTypeManager::saveResource(const ResourceBuildDescriptor& desc, const ScopedPath& dst)
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

std::unique_ptr<ResourceLoadDescriptor> MaterialTypeManager::makeResourceLoadDescriptor()
{
	return std::make_unique<MaterialLoadDescriptor>();
}

std::unique_ptr<ResourceBuildDescriptor> MaterialTypeManager::makeResourceBuildDescriptor()
{
	return std::make_unique<MaterialCreateDescriptor>();
}

ResourceWrapper<Resource> MaterialTypeManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	auto materialDesc = dynamic_cast<const MaterialLoadDescriptor*>(&desc);
	if (!materialDesc)
	{
		logError("Invalid Descriptor specified.");
		return ResourceWrapper<Resource>::empty;
	}

	return Material::load(desc.sourcePath, *materialDesc);
}

void MaterialTypeManager::parse(ResourceLoadDescriptor& desc)
{
}

void MaterialTypeManager::parse(ResourceBuildDescriptor& desc)
{
	auto materialDesc = dynamic_cast<MaterialCreateDescriptor*>(&desc);
	if (!materialDesc)
	{
		logError("Invalid Descriptor specified.");
		return;
	}

	if (!materialDesc->data.isParsed())
	{
		MaterialDataParser::parse(materialDesc->data);
	}
}