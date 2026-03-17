#include "ShaderManager.h"

#include "render/Shader.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"

#include <filesystem>

Ref<Asset> ShaderTypeManager::createAsset(const AssetBuildDescriptor& assetDesc, const ResourceBuildDescriptor& resourceDesc)
{
	return createRef< ShaderAsset>();
}

Ref<Asset> ShaderTypeManager::deserializeAsset(const nlohmann::json& j)
{
	auto asset = createRef<ShaderAsset>();
	asset->deserialize(j);
	return asset;
}

bool ShaderTypeManager::importAsset(const std::string& src, ImportNode& result)
{
	// Shader import not implemented
	return false;
}

bool ShaderTypeManager::saveResource(const ResourceBuildDescriptor& desc, const ScopedPath& dst)
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

void ShaderTypeManager::parse(ResourceBuildDescriptor& desc)
{
}

