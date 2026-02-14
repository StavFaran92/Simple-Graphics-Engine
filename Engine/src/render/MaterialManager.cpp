#include "MaterialManager.h"

#include "render/Material.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"

#include <fstream>
#include <cereal/archives/json.hpp>

void MaterialTypeManager::Saver::save(const AssetRecord& record)
{
	// TODO: save is called before the asset/resource exists in the system.
	// Original: cereal JSON serialize *AssetHandle<MaterialAsset>(m_uuid).resource().get() to record.fullFilePath
}

void MaterialTypeManager::Importer::import(const AssetRecord& record)
{
	// Material import is a no-op
}

Asset* MaterialTypeManager::Factory::create(AssetCreateDescriptor& desc)
{
	return new MaterialAsset(desc);
}
