#include "ShaderManager.h"

#include "render/Shader.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"

#include <filesystem>

void ShaderTypeManager::Saver::save(const AssetRecord& record)
{
	std::filesystem::copy_file(record.sourcePath, record.fullFilePath, std::filesystem::copy_options::overwrite_existing);
}

void ShaderTypeManager::Importer::import(const AssetRecord& record)
{
	std::filesystem::copy_file(record.sourcePath, record.fullFilePath, std::filesystem::copy_options::overwrite_existing);
}

Asset* ShaderTypeManager::Factory::create(AssetCreateDescriptor& desc)
{
	return new ShaderAsset(desc);
}
