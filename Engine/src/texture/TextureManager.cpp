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
	return std::filesystem::copy_file(src, dst.absolute(), std::filesystem::copy_options::overwrite_existing);
}

bool TextureTypeManager::saveResource(const ResourceCreateDescriptor& desc, const ScopedPath& dst)
{
	auto textureDesc = dynamic_cast<const TextureCreateDescriptor*>(&desc);
	if (!textureDesc)
	{
		logError("Invalid Descriptor specified.");
		return false;
	}

	const TextureData& data = textureDesc->textureData;

	if (data.type == TextureType::FLOAT)
	{
		EXRLoader::saveSingleChannelEXR(dst.absolute().string(), 
			data.width, 
			data.height, 
			(const float*)data.data);
	}
	else
	{
		STBIHelper::writeToPNG(dst.absolute().string(), 
			data.width, 
			data.height, 
			data.channels, 
			data.data, 
			data.width * data.channels);


	}

	return true;
}

ResourceLoadDescriptor* TextureTypeManager::makeResourceLoadDescriptor()
{
	return new TextureLoadDescriptor();
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