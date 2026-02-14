#include "AnimationManager.h"

#include "animation/Animation.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"

#include <filesystem>
#include <stdexcept>

void AnimationTypeManager::Saver::save(const AssetRecord& record)
{
	// TODO: implement animation save
	throw std::runtime_error("Animation save not yet implemented!");
}

void AnimationTypeManager::Importer::import(const AssetRecord& record)
{
	std::filesystem::copy_file(record.sourcePath, record.fullFilePath, std::filesystem::copy_options::overwrite_existing);
}

Asset* AnimationTypeManager::Factory::create(AssetCreateDescriptor& desc)
{
	return new AnimationAsset(desc);
}
