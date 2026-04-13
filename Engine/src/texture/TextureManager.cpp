#include "TextureManager.h"

#include "texture/Texture.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"
#include "utils/EXRLoader.h"
#include "utils/STBIHelper.h"
#include "texture/TextureBinaryLoader.h"

#include <filesystem>

Ref<Asset> TextureTypeManager::createAsset(const AssetBuildDescriptor& assetDesc, const ResourceBuildDescriptor& resourceDesc)
{
	return createRef< TextureAsset>();
}

Ref<Asset> TextureTypeManager::deserializeAsset(const nlohmann::json& j)
{
	auto asset = createRef<TextureAsset>();
	asset->deserialize(j);
	return asset;
}

bool TextureTypeManager::importAsset(const ResourceLoadDescriptor& loadDesc, ImportNode& result)
{
	// TODO fix, what the hell is going on here?
	TextureData textureData;
	Texture::extractTextureDataFromFile(loadDesc.sourcePath, textureData);

	std::filesystem::path path(loadDesc.sourcePath);
	result.name = path.filename().stem().string();
	result.assetDesc.name = path.filename().stem().string();
	result.assetDesc.aType = AssetType::TEXTURE;
	//result.assetDesc.sourcePath = src;
	auto texCreateDesc = result.emplaceCreateDesc<TextureCreateDescriptor>();
	texCreateDesc->textureData = textureData;

	return true;
}

bool TextureTypeManager::saveResource(const ResourceBuildDescriptor& desc, const ScopedPath& dst)
{
	auto textureDesc = dynamic_cast<const TextureCreateDescriptor*>(&desc);
	if (!textureDesc)
	{
		logError("Invalid Descriptor specified.");
		return false;
	}

	const TextureData& data = textureDesc->textureData;

	TextureBinaryLoader::save(data, dst.absolute().string());

	return true;
}

std::unique_ptr<ResourceLoadDescriptor> TextureTypeManager::makeResourceLoadDescriptor()
{
	return std::make_unique<TextureLoadDescriptor>();
}

std::unique_ptr<ResourceBuildDescriptor> TextureTypeManager::makeResourceBuildDescriptor()
{
	return std::make_unique<TextureCreateDescriptor>();
}

ResourceWrapper<Resource> TextureTypeManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	auto textureDesc = dynamic_cast<const TextureLoadDescriptor*>(&desc);
	if (!textureDesc)
	{
		logError("Invalid Descriptor specified.");
		return ResourceWrapper<Resource>::empty;
	}

	TextureData textureData;
	TextureBinaryLoader::load(desc.sourcePath, textureData);
	return Texture::createTexture(textureData);
}

void TextureTypeManager::parse(ResourceLoadDescriptor& desc)
{
}

void TextureTypeManager::parse(ResourceBuildDescriptor& desc)
{
}

void TextureTypeManager::extractResourceData(const ResourceWrapper<Resource>& resource, ResourceBuildDescriptor& outDesc)
{
	auto texture = resource.as<Texture>();
	if (!texture)
	{
		logError("TextureTypeManager::extractResourceData called with non-texture resource");
		throw std::runtime_error("Invalid resource type for TextureTypeManager::extractResourceData");
	}

	// Ensure descriptor type
	auto* texDesc = dynamic_cast<TextureCreateDescriptor*>(&outDesc);
	if (!texDesc)
	{
		throw std::runtime_error("Invalid descriptor type for TextureTypeManager::extractResourceData");
	}

	texture->download();
	texDesc->textureData = texture->getData();
}