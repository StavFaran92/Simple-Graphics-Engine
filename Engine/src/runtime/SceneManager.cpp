#include "SceneManager.h"

#include "runtime/Scene.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"

#include <fstream>
#include <cereal/archives/json.hpp>

void SceneTypeManager::Saver::save(const AssetRecord& record)
{
	// TODO: save is called before the asset/resource exists in the system.
	// Original: Archiver::serializeScene(resource) -> cereal JSON to record.fullFilePath
}

void SceneTypeManager::Importer::import(const AssetRecord& record)
{
	// Scene import is a no-op
}

Asset* SceneTypeManager::Factory::create(AssetCreateDescriptor& desc)
{
	return new SceneAsset(desc);
}
