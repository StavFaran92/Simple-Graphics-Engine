#include "SceneManager.h"

#include "runtime/Scene.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"

#include <fstream>
#include <cereal/archives/json.hpp>



Ref<Asset> SceneTypeManager::createAsset(AssetCreateDescriptor& desc)
{
	return createRef<SceneAsset>();
}

Ref<Asset> SceneTypeManager::deserializeAsset(const nlohmann::json& j)
{
	auto asset = createRef<SceneAsset>();
	asset->deserialize(j);
	return asset;
}

bool SceneTypeManager::importAsset(const std::string& src, ImportNode& result)
{
	// Scene import not implemented
	return false;
}

bool SceneTypeManager::saveResource(const ResourceCreateDescriptor& desc, const ScopedPath& dst)
{
	auto sceneDesc = dynamic_cast<const SceneCreateDescriptor*>(&desc);
	if (!sceneDesc)
	{
		logError("Invalid Descriptor specified.");
		return false;
	}

	std::ofstream os(dst.absolute());
	cereal::JSONOutputArchive oarchive(os);

	try
	{
		oarchive(sceneDesc->data);
	}
	catch (const cereal::Exception& e)
	{
		logError("Serialization Error occured: {}", e.what());
	}

	return true;
}

ResourceLoadDescriptor* SceneTypeManager::makeResourceLoadDescriptor()
{
	return nullptr;
}

ResourceWrapper<Resource> SceneTypeManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	return ResourceWrapper<Resource>();
}

void SceneTypeManager::parse(ResourceLoadDescriptor& desc)
{
}

void SceneTypeManager::parse(ResourceCreateDescriptor& desc)
{
}

