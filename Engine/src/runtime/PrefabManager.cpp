#include "PrefabManager.h"

#include "runtime/Prefab.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"

#include <fstream>
#include <cereal/archives/json.hpp>

Ref<Asset> PrefabTypeManager::createAsset(const AssetBuildDescriptor& assetDesc, const ResourceBuildDescriptor& resourceDesc)
{
	return createRef< PrefabAsset>();
}

Ref<Asset> PrefabTypeManager::deserializeAsset(const nlohmann::json& j)
{
	auto asset = createRef<PrefabAsset>();
	asset->deserialize(j);
	return asset;
}

bool PrefabTypeManager::importAsset(const ResourceLoadDescriptor& loadDesc, ImportNode& result)
{
	// Prefab import not implemented
	return false;
}

bool PrefabTypeManager::saveResource(const ResourceBuildDescriptor& desc, const ScopedPath& dst)
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

std::unique_ptr<ResourceLoadDescriptor> PrefabTypeManager::makeResourceLoadDescriptor()
{
	return std::make_unique<PrefabLoadDescriptor>();
}

std::unique_ptr<ResourceBuildDescriptor> PrefabTypeManager::makeResourceBuildDescriptor()
{
	return std::make_unique<PrefabCreateDescriptor>();
}

ResourceWrapper<Resource> PrefabTypeManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	auto prefabDesc = dynamic_cast<const PrefabLoadDescriptor*>(&desc);
	if (!prefabDesc)
	{
		logError("Invalid Descriptor specified.");
		return ResourceWrapper<Resource>::empty;
	}

	return Prefab::load(desc.sourcePath, *prefabDesc);
}

void PrefabTypeManager::parse(ResourceLoadDescriptor& desc)
{
}

void PrefabTypeManager::parse(ResourceBuildDescriptor& desc)
{
}

