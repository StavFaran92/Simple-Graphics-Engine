#include "AnimationManager.h"

#include "animation/Animation.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"

#include <filesystem>
#include <stdexcept>

Asset* AnimationTypeManager::createAsset(AssetCreateDescriptor& desc)
{
	return new AnimationAsset(desc);
}

bool AnimationTypeManager::importAsset(const std::string& src, const ScopedPath& dst, ImportNode& result)
{
	if (!std::filesystem::copy_file(src, dst.absolute(), std::filesystem::copy_options::overwrite_existing))
	{
		return false;
	}

	std::filesystem::path path(src);
	result.name = path.filename().stem().string();
	result.createDescriptor.aType = AssetType::ANIMATION;
	result.createDescriptor.sourcePath = src;
	result.createDescriptor.makeResourceLoadDescriptor<AnimationLoadDescriptor>()->sourcePath = src;

	return true;
}

bool AnimationTypeManager::saveResource(const ResourceCreateDescriptor& desc, const ScopedPath& dst)
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

void AnimationTypeManager::parse(ResourceCreateDescriptor& desc)
{
}
