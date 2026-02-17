#include "TextureManager.h"

#include "texture/Texture.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"
#include "utils/EXRLoader.h"
#include "utils/STBIHelper.h"

#include <filesystem>

Asset* TextureTypeManager::createAsset(AssetCreateDescriptor& desc)
{
	return new TextureAsset(desc);
}

bool TextureTypeManager::importAsset(const std::string& src, const ScopedPath& dst)
{
	return false;
}

bool TextureTypeManager::saveResource(const ResourceCreateDescriptor& desc, const ScopedPath& dst)
{
	return false;
}

ResourceLoadDescriptor* TextureTypeManager::makeResourceLoadDescriptor()
{
	return nullptr;
}

ResourceWrapper<Resource> TextureTypeManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	return ResourceWrapper<Resource>();
}

void TextureTypeManager::parse(ResourceLoadDescriptor& desc)
{
}

void TextureTypeManager::parse(ResourceCreateDescriptor& desc)
{
}