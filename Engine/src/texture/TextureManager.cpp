#include "TextureManager.h"

#include "texture/Texture.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"
#include "utils/EXRLoader.h"
#include "utils/STBIHelper.h"

#include <filesystem>

void TextureTypeManager::Saver::save(const AssetRecord& record)
{
	// TODO: save is called before the asset/resource exists in the system,
	// so we can't do AssetHandle<TextureAsset>(uuid).resource() here yet.
	// Original logic: if float texture -> EXRLoader::saveSingleChannelEXR, else STBIHelper::writeToPNG
}

void TextureTypeManager::Importer::import(const AssetRecord& record)
{
	std::filesystem::copy_file(record.sourcePath, record.fullFilePath, std::filesystem::copy_options::overwrite_existing);
}

Asset* TextureTypeManager::Factory::create(AssetCreateDescriptor& desc)
{
	return new TextureAsset(desc);
}
