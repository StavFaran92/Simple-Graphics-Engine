#include "ShaderManager.h"

#include "render/Shader.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"

#include <filesystem>

Asset* ShaderTypeManager::createAsset(AssetCreateDescriptor& desc)
{
	return new ShaderAsset(desc);
}

bool ShaderTypeManager::importAsset(const std::string& src, const ScopedPath& dst)
{
	return false;
}

bool ShaderTypeManager::saveResource(const ResourceCreateDescriptor& desc, const ScopedPath& dst)
{
	return false;
}

ResourceLoadDescriptor* ShaderTypeManager::makeResourceLoadDescriptor()
{
	return nullptr;
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

