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
	return false;
}

bool MeshGroupTypeManager::saveResource(const ResourceCreateDescriptor& desc, const ScopedPath& dst)
{
	return false;
}

ResourceLoadDescriptor* MeshGroupTypeManager::makeResourceLoadDescriptor()
{
	return nullptr;
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