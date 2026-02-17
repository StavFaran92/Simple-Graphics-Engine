#include "PrefabManager.h"

#include "runtime/Prefab.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"

#include <fstream>
#include <cereal/archives/json.hpp>

void PrefabTypeManager::saveAsset(const AssetRecord& record)
{
	// TODO: save is called before the asset/resource exists in the system.
	// Original: cereal JSON serialize *AssetHandle<PrefabAsset>(m_uuid).resource().get() to record.fullFilePath
}

void PrefabTypeManager::importAsset(const AssetRecord& record)
{
	// Prefab import is a no-op
}

Asset* PrefabTypeManager::createAsset(AssetCreateDescriptor& desc)
{
	return new PrefabAsset(desc);
}
