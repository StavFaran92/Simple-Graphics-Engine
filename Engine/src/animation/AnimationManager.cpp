#include "AnimationManager.h"

#include "animation/Animation.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"

#include "animation/AnimationLoader.h"

#include <filesystem>
#include <stdexcept>

Ref<Asset> AnimationTypeManager::createAsset(const AssetBuildDescriptor& assetDesc, const ResourceBuildDescriptor& resourceDesc)
{
	return createRef< AnimationAsset>();
}

Ref<Asset> AnimationTypeManager::deserializeAsset(const nlohmann::json& j)
{
	auto asset = createRef<AnimationAsset>();
	asset->deserialize(j);
	return asset;
}

bool AnimationTypeManager::importAsset(const std::string& src, ImportNode& result)
{
	AnimationLoader::AnimationInfo animInfo;
	if (!Engine::get()->getSubSystem<AnimationLoader>()->parseAnimation(src, animInfo))
	{
		logWarning("Failed to import asset: {}", src);
		return false;
	}

	std::filesystem::path path(src);
	result.name = path.filename().stem().string();
	result.assetDesc.aType = AssetType::ANIMATION;
	auto rootAnimDesc = result.emplaceCreateDesc<AnimationCreateDescriptor>();
	//rootAnimDesc->data = modelInfo.meshDataList;

	return true;
}

bool AnimationTypeManager::saveResource(const ResourceBuildDescriptor& desc, const ScopedPath& dst)
{
	throw std::runtime_error("Animation save not yet implemented!");
}

ResourceLoadDescriptor* AnimationTypeManager::makeResourceLoadDescriptor()
{
	return nullptr;
}

ResourceWrapper<Resource> AnimationTypeManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	return ResourceWrapper<Resource>();
}

void AnimationTypeManager::parse(ResourceLoadDescriptor& desc)
{
}

void AnimationTypeManager::parse(ResourceBuildDescriptor& desc)
{
}
