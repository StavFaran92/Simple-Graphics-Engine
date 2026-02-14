#include "MeshGroupManager.h"

#include "geometry/MeshGroup.h"
#include "geometry/ModelImporter.h"
#include "geometry/MeshExporter.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"

void MeshGroupTypeManager::Saver::save(const AssetRecord& record)
{
	// TODO: save is called before the asset exists, can't use AssetHandle yet.
	// Original: MeshExporter::exportMesh(AssetHandle<MeshGroupAsset>(m_uuid));
}

void MeshGroupTypeManager::Importer::import(const AssetRecord& record)
{
	// TODO: ModelImporter::copyFiles takes non-const AssetRecord&, need to reconcile
	// Original: Engine::get()->getSubSystem<ModelImporter>()->copyFiles(fileLocation, aInfo);
}

Asset* MeshGroupTypeManager::Factory::create(AssetCreateDescriptor& desc)
{
	return new MeshGroupAsset(desc);
}
