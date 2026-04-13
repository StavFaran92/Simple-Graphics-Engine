#include "AnimationManager.h"

#include "animation/Animation.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"

#include "animation/AnimationLoader.h"

#include "animation/AnimationBinaryLoader.h"
#include "core/Factory.h"

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

bool AnimationTypeManager::importAsset(const ResourceLoadDescriptor& loadDesc, ImportNode& result)
{
	AnimationLoader::AnimationInfo animInfo;
	if (!Engine::get()->getSubSystem<AnimationLoader>()->parseAnimation(loadDesc.sourcePath, animInfo))
	{
		logWarning("Failed to import asset: {}", loadDesc.sourcePath);
		return false;
	}

	std::filesystem::path path(loadDesc.sourcePath);
	result.name = path.filename().stem().string();
	result.assetDesc.aType = AssetType::ANIMATION;
	auto rootAnimDesc = result.emplaceCreateDesc<AnimationCreateDescriptor>();
	
	rootAnimDesc->data.bones = animInfo.m_bones;
	rootAnimDesc->data.name = animInfo.m_name;
	rootAnimDesc->data.duration = animInfo.m_duration;
	rootAnimDesc->data.ticksPerSecond = animInfo.m_ticksPerSecond;
	rootAnimDesc->data.nodes = animInfo.m_nodes;
	//TODO complete nodes

	return true;
}

bool AnimationTypeManager::saveResource(const ResourceBuildDescriptor& desc, const ScopedPath& dst)
{
	auto animDesc = dynamic_cast<const AnimationCreateDescriptor*>(&desc);
	if (!animDesc)
	{
		logError("Invalid Descriptor specified.");
		return false;
	}

	const AnimationData& data = animDesc->data;

	AnimationBinaryLoader::save(data, dst.absolute().string());

	return true;
}

std::unique_ptr<ResourceLoadDescriptor> AnimationTypeManager::makeResourceLoadDescriptor()
{
	return std::make_unique<AnimationLoadDescriptor>();
}

std::unique_ptr<ResourceBuildDescriptor> AnimationTypeManager::makeResourceBuildDescriptor()
{
	return std::make_unique<AnimationCreateDescriptor>();
}

ResourceWrapper<Resource> AnimationTypeManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	AnimationData animationData;
	AnimationBinaryLoader::load(desc.sourcePath, animationData);

	ResourceWrapper<Animation> animation = Factory<Animation>::create();
	animation->build(animationData);
	return animation;
}

void AnimationTypeManager::parse(ResourceLoadDescriptor& desc)
{
}

void AnimationTypeManager::parse(ResourceBuildDescriptor& desc)
{
}
