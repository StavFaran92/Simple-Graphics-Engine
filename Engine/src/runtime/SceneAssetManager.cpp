#include "SceneAssetManager.h"

#include "runtime/Scene.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"

#include <fstream>
#include <cereal/archives/json.hpp>

#include "component/ComponentSerializer.h"

#include "entt/entt.hpp"

Ref<Asset> SceneAssetManager::createAsset(const AssetBuildDescriptor& assetDesc, const ResourceBuildDescriptor& resourceDesc)
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

bool SceneAssetManager::saveResource(const ResourceBuildDescriptor& desc, const ScopedPath& dst)
{
	auto sceneDesc = dynamic_cast<const SceneCreateDescriptor*>(&desc);
	if (!sceneDesc)
	{
		logError("Invalid Descriptor specified.");
		return false;
	}

	//std::ofstream os(path);
	//cereal::JSONOutputArchive oarchive(os);

	//try
	//{
	//	oarchive(sceneDesc->data);
	//}
	//catch (const cereal::Exception& e)
	//{
	//	logError("Serialization Error occured: {}", e.what());
	//}

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

std::unique_ptr<ResourceLoadDescriptor> SceneAssetManager::makeResourceLoadDescriptor()
{
	return std::make_unique<SceneLoadDescriptor>();
}

std::unique_ptr<ResourceBuildDescriptor> SceneAssetManager::makeResourceBuildDescriptor()
{
	return std::make_unique<SceneCreateDescriptor>();
}

ResourceWrapper<Resource> SceneAssetManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	auto sceneDesc = dynamic_cast<const SceneLoadDescriptor*>(&desc);
	if (!sceneDesc)
	{
		logError("Invalid Descriptor specified.");
		return ResourceWrapper<Resource>::empty;
	}

	return Scene::load(desc.sourcePath, *sceneDesc);
}

void SceneAssetManager::parse(ResourceLoadDescriptor& desc)
{
}

void SceneAssetManager::parse(ResourceBuildDescriptor& desc)
{
	//auto sceneDesc = dynamic_cast<SceneCreateDescriptor*>(&desc);
	//if (!sceneDesc)
	//{
	//	logError("Invalid Descriptor specified.");
	//	return;
	//}

	//std::stringstream& source = sceneDesc->data.registryStream;

	//if (source.str().empty())
	//{
	//	SGE_Regsitry sgeRegistry{};
	//	source = sgeRegistry.toStream();
	//}
}
