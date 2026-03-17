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

bool TextureTypeManager::importAsset(const std::string& src, ImportNode& result)
{
	// TODO fix, what the hell is going on here?
	TextureData textureData;
	Texture::extractTextureDataFromFile(src, textureData);

	std::filesystem::path path(src);
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

void TextureTypeManager::parse(ResourceBuildDescriptor& desc)
{
}