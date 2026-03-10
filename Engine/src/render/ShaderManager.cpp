#include "ShaderManager.h"

#include "render/Shader.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"

#include <filesystem>

Ref<Asset> ShaderTypeManager::createAsset(AssetCreateDescriptor& desc)
{
	return createRef< ShaderAsset>();
}

bool ShaderTypeManager::importAsset(const std::string& src, ImportNode& result)
{
	// Shader import not implemented
	return false;
}

bool ShaderTypeManager::saveResource(const ResourceCreateDescriptor& desc, const ScopedPath& dst)
{
	return false;
}

ResourceLoadDescriptor* ShaderTypeManager::makeResourceLoadDescriptor()
{
	return new ShaderLoadDescriptor();
}

ResourceWrapper<Resource> ShaderTypeManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	return ResourceWrapper<Resource>();
}

void ShaderTypeManager::parse(ResourceLoadDescriptor& desc)
{
}

void ShaderTypeManager::parse(ResourceCreateDescriptor& desc)
{
}

