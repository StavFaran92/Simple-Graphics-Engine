#include "MeshGroupManager.h"

#include "geometry/MeshGroup.h"
#include "geometry/ModelImporter.h"
#include "geometry/MeshExporter.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"

Asset* MeshGroupTypeManager::createAsset(AssetCreateDescriptor& desc)
{
	return new MeshGroupAsset(desc);
}

bool MeshGroupTypeManager::importAsset(const std::string& src, const ScopedPath& dst)
{
	// we want to do the most work we can do here, so it will not need to happen during load
	// we open the file and parse all its inner data
	// and so at the end what imported is
	// array of meshes
	// textures
	// materials
	// skeleton
	// etc
	// so i may need to alter the signature to store dependant assets
	// or at least connect them somehow.
	// so what i will do is open the file using assimp 
	// and bring mesh currently as OBJ.
	// and for each dependant assets currently bring them also, no connection ATM

	std::vector<ScopedPath> importedFiles;
	Engine::get()->getSubSystem<ModelImporter>()->importModel(src, dst, importedFiles);

	return false;
}

bool MeshGroupTypeManager::saveResource(const ResourceCreateDescriptor& desc, const ScopedPath& dst)
{
	auto meshGroupDesc = dynamic_cast<const MeshGroupCreateDescriptor*>(&desc);
	if (!meshGroupDesc)
	{
		logError("Invalid Descriptor specified.");
		return false;
	}

	const MeshData& data = meshGroupDesc->data;

	MeshExporter::exportMesh(data, dst.absolute().string());

	return true;
}

ResourceLoadDescriptor* MeshGroupTypeManager::makeResourceLoadDescriptor()
{
	return new MeshGroupLoadDescriptor();
}

ResourceWrapper<Resource> MeshGroupTypeManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	return ResourceWrapper<Resource>();
}

void MeshGroupTypeManager::parse(ResourceLoadDescriptor& desc)
{
}

void MeshGroupTypeManager::parse(ResourceCreateDescriptor& desc)
{
}