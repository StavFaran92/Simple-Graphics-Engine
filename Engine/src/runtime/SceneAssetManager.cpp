#include "SceneAssetManager.h"

#include "runtime/Scene.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"

#include <fstream>
#include <cereal/archives/json.hpp>



Ref<Asset> SceneAssetManager::createAsset(AssetCreateDescriptor& desc)
{
	return createRef<SceneAsset>();
}

Ref<Asset> SceneAssetManager::deserializeAsset(const nlohmann::json& j)
{
	auto asset = createRef<SceneAsset>();
	asset->deserialize(j);
	return asset;
}

bool SceneAssetManager::importAsset(const std::string& src, ImportNode& result)
{
	// Scene import not implemented
	return false;
}

bool SceneAssetManager::saveResource(const ResourceCreateDescriptor& desc, const ScopedPath& dst)
{
	auto sceneDesc = dynamic_cast<const SceneCreateDescriptor*>(&desc);
	if (!sceneDesc)
	{
		logError("Invalid Descriptor specified.");
		return false;
	}

	auto path = dst.absolute();
	std::ofstream os(path);
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

ResourceLoadDescriptor* SceneAssetManager::makeResourceLoadDescriptor()
{
	return new SceneLoadDescriptor();
}

ResourceWrapper<Resource> SceneAssetManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	return ResourceWrapper<Resource>();
}

void SceneAssetManager::parse(ResourceLoadDescriptor& desc)
{
}

void SceneAssetManager::parse(ResourceCreateDescriptor& desc)
{
}
