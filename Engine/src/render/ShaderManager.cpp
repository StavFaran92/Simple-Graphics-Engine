#include "ShaderManager.h"

#include "render/Shader.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "render/ShaderLoader.h"

#include <filesystem>
#include <fstream>

Ref<Asset> ShaderTypeManager::createAsset(const AssetBuildDescriptor& assetDesc, const ResourceBuildDescriptor& resourceDesc)
{
	auto shaderDesc = dynamic_cast<const ShaderCreateDescriptor*>(&resourceDesc);
	if (!shaderDesc)
	{
		logError("Invalid Descriptor specified.");
		return nullptr;
	}
	return createRef< ShaderAsset>(*shaderDesc);
}

Ref<Asset> ShaderTypeManager::deserializeAsset(const nlohmann::json& j)
{
	auto asset = createRef<ShaderAsset>();
	asset->deserialize(j);
	return asset;
}

bool ShaderTypeManager::importAsset(const std::string& src, ImportNode& result)
{
	std::filesystem::path path(src);
	result.name = path.filename().stem().string();
	result.assetDesc.aType = AssetType::SHADER;
	auto shaderCreateDesc = result.emplaceCreateDesc<ShaderCreateDescriptor>();

	std::string code = Engine::get()->getShaderLoader()->readShader(src);
	shaderCreateDesc->code = code;

	return true;
}

bool ShaderTypeManager::saveResource(const ResourceBuildDescriptor& desc, const ScopedPath& dst)
{
	auto shaderDesc = dynamic_cast<const ShaderCreateDescriptor*>(&desc);
	if (!shaderDesc)
	{
		logError("Invalid Descriptor specified.");
		return false;
	}

	std::ofstream os(dst.absolute());
	if (!os.is_open())
		return false;

	os << shaderDesc->code;

	os.close();

	return true;
}

std::unique_ptr<ResourceLoadDescriptor> ShaderTypeManager::makeResourceLoadDescriptor()
{
	return std::make_unique<ShaderLoadDescriptor>();
}

std::unique_ptr<ResourceBuildDescriptor> ShaderTypeManager::makeResourceBuildDescriptor()
{
	return std::make_unique<ShaderCreateDescriptor>();
}

ResourceWrapper<Resource> ShaderTypeManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	auto shaderDesc = dynamic_cast<const ShaderLoadDescriptor*>(&desc);
	if (!shaderDesc)
	{
		logError("Invalid Descriptor specified.");
		return false;
	}

	return Shader::load(desc.sourcePath, *shaderDesc);
}

void ShaderTypeManager::parse(ResourceLoadDescriptor& desc)
{
}

void ShaderTypeManager::parse(ResourceBuildDescriptor& desc)
{
	auto shaderDesc = dynamic_cast<ShaderCreateDescriptor*>(&desc);
	if (!shaderDesc)
	{
		logError("Invalid Descriptor specified.");
		return;
	}

	shaderDesc->code = std::string(R"(

#vert

void vert(inout vec3 aPos, inout vec3 aNorm)
{
}

#frag

void frag(inout vec3 color, 
	inout vec3 normal, 
	inout float metallic, 
	inout float roughness, 
	inout float ao)
{      
    color = vec3(1.0f, 0.0f, 0.0f);
})");
}

