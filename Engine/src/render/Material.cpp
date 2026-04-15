#include "render/Material.h"

#include "core/Logger.h"
#include <GL\glew.h>

#include "core/Engine.h"
#include "runtime/Context.h"

#include "memory/AssetLoader.h"
#include "core/Factory.h"
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/archives/json.hpp>
#include <iostream>
#include <fstream>
#include <regex>
#include "memory/BuiltInAssets.h"
#include "memory/BuiltInResources.h"
#include "render/ShadersInfo.h"
#include "memory/Assets.h"

#include <filesystem>
#include "memory/AssetHandle.h"
#include "render/MaterialDataParser.h"
#include "texture/Texture.h"
#include "texture/TextureSampler.h"

void useSamplerInShader(const std::string& name, std::shared_ptr<TextureSampler> sampler, ShaderResourceRef& shader, int slot)
{
	// if texture is empty use dummy texture
	TextureAssetRef texture;
	if (!sampler || sampler->texture.isEmpty())
	{
		texture = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_WHITE); // maybe use disgusting pink texture?
	}
	else
	{
		texture = sampler->texture;
	}

	texture.resource()->setSlot(slot);
	texture.resource()->bind();

	// set sampler2D (e.g. material.diffuse3 to the currently active texture unit)
	shader->setUniformValue(name + ".texture", slot);

	shader->setUniformValue(name + ".isActive", sampler->isActive);
	shader->setUniformValue(name + ".xOffset", sampler ? sampler->xOffset : 0.0f);
	shader->setUniformValue(name + ".yOffset", sampler ? sampler->yOffset : 0.0f);
	shader->setUniformValue(name + ".xScale", sampler ? sampler->xScale : 1.0f);
	shader->setUniformValue(name + ".yScale", sampler ? sampler->yScale : 1.0f);
	shader->setUniformValue(name + ".channelMaskR", sampler ? sampler->channelMaskR : 1);
	shader->setUniformValue(name + ".channelMaskG", sampler ? (sampler->channelCount > 1 ? sampler->channelMaskG : 0) : 0);
	shader->setUniformValue(name + ".channelMaskB", sampler ? (sampler->channelCount > 2 ? sampler->channelMaskB : 0) : 0);
	shader->setUniformValue(name + ".channelMaskA", sampler ? (sampler->channelCount > 3 ? sampler->channelMaskA : 0) : 0);
}

void Material::use()
{
	ShaderResourceRef shader = getActiveShader();

	if (shader.isEmpty())
	{
		logWarning("Material {} shader is invalid.", m_name);
		return;
	}

	shader->use();

	int slot = 0;

	// Set samplers
	for (const auto& [name, sampler] : m_samplers)
	{
		useSamplerInShader(name, sampler, shader, slot);
		slot++;
	}

	// Set uniforms
	for (const auto& [name, value] : m_uniformProperties)
	{
		shader->setUniformValue(name, value);
	}
}

void Material::release()
{
	for (auto i = 0; i < 3; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

std::shared_ptr<TextureSampler> Material::getSampler(const std::string& name)
{
	auto it = m_samplers.find(name);
	return it != m_samplers.end() ? it->second : nullptr;
}

void Material::setSampler(const std::string& name, std::shared_ptr<TextureSampler> sampler)
{
	m_samplers[name] = sampler;
}

Value Material::getUniformValue(const std::string& name)
{
	auto it = m_uniformProperties.find(name);
	return it != m_uniformProperties.end() ? it->second : Value{};
}

MaterialResourceRef Material::create(MaterialRenderMode renderMode)
{
	auto mat = Factory<Material>::create();
	mat->m_renderMode = renderMode;
	return mat;
}

MaterialResourceRef Material::clone(bool isEngineOwned) const
{
	auto newMaterial = Material::create(m_renderMode);

	newMaterial->m_samplers = m_samplers;
	newMaterial->m_uniformProperties = m_uniformProperties;
	newMaterial->m_customShader = m_customShader;
	newMaterial->m_name = m_name;

	return newMaterial;
}

ShaderResourceRef Material::getActiveShader() const
{
	if (m_renderMode != MaterialRenderMode::Custom)
	{
		return MaterialDataParser::getShaderFromRenderMode(m_renderMode);
	}
	else
	{
		if (m_customShader.isEmpty())
		{
			logWarning("Invalid custom shader for material {}, using fallback.", m_name);
			return BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_FORWARD_PBR);
		}
		return m_customShader;
	}
}

void Material::setTexture(const std::string& name, const TextureResourceRef& texture)
{
}

void Material::setUniformValue(const std::string& name, const Value& v)
{
	m_uniformProperties[name] = v;
}

std::string Material::getName() const
{
	return m_name;
}

MaterialRenderMode Material::getRenderMode() const
{
	return m_renderMode;
}

MaterialResourceRef Material::load(const std::string& fileLocation, MaterialLoadDescriptor desc)
{
	desc.sourcePath = fileLocation;
	std::string filepath = desc.sourcePath;
	//auto projectDir = Engine::get()->getProjectDirectory();
	//filepath = projectDir + filepath;
	std::ifstream is(filepath);
	cereal::JSONInputArchive iarchive(is);
	MaterialData materialData;
	//MaterialResourceRef material = Factory<Material>::create();

	try
	{
		iarchive(materialData);
		MaterialResourceRef material = Factory<Material>::create();
		material->m_renderMode = materialData.getMaterialRenderMode();
		material->m_customShader = materialData.getCustomShader().resource();
		material->m_name = materialData.name;
		return material;

	}
	catch (const cereal::Exception& e)
	{
		logError("Deserialization Error occured: {}", e.what());
	}

	return MaterialResourceRef::empty;
}

//////////////////////////
// Asset
//////////////////////////

MaterialAsset::MaterialAsset(const MaterialCreateDescriptor& desc)
{
	data = desc.data;
}

void MaterialAsset::bindDependency(const std::string& slot, UUID dependency)
{
	// Map slot names to shader property names
	std::string shaderPropertyName;
	int samplerChannels = 1;
	if (slot == "ALBEDO" || slot == SHADER_PROPERTY_PBR_SAMPLER_ALBEDO)
	{
		shaderPropertyName = SHADER_PROPERTY_PBR_SAMPLER_ALBEDO;
		samplerChannels = 3;
	}
	else if (slot == "NORMAL" || slot == SHADER_PROPERTY_PBR_SAMPLER_NORMAL)
	{
		shaderPropertyName = SHADER_PROPERTY_PBR_SAMPLER_NORMAL;
		samplerChannels = 3;
	}
	else if (slot == "ROUGHNESS" || slot == SHADER_PROPERTY_PBR_SAMPLER_ROUGHNESS)
	{
		shaderPropertyName = SHADER_PROPERTY_PBR_SAMPLER_ROUGHNESS;
		samplerChannels = 1;
	}
	else if (slot == "METALLIC" || slot == SHADER_PROPERTY_PBR_SAMPLER_METALLIC)
	{
		shaderPropertyName = SHADER_PROPERTY_PBR_SAMPLER_METALLIC;
		samplerChannels = 1;
	}
	else if (slot == "AO" || slot == SHADER_PROPERTY_PBR_SAMPLER_AO)
	{
		shaderPropertyName = SHADER_PROPERTY_PBR_SAMPLER_AO;
		samplerChannels = 1;
	}
	else
	{
		// Use slot name directly if it doesn't match known patterns
		shaderPropertyName = slot;
	}

	// Cast dependency to TextureAsset
	TextureAssetRef textureAsset(dependency);
	if (textureAsset.isEmpty())
	{
		logWarning("MaterialAsset::bindDependency: Dependency is not a TextureAsset for slot '{}'", slot);
		return;
	}
	
	std::shared_ptr<TextureSampler> sampler = std::make_shared<TextureSampler>(samplerChannels);
	sampler->isActive = true;
	sampler->texture = textureAsset;
	data.setSampler(shaderPropertyName, sampler);
}

void MaterialAsset::fillData(ResourceWrapper<Resource> resource)
{
	auto materialResource = resource.as<Material>();
	materialResource->m_name = data.name;
	materialResource->m_renderMode = data.getMaterialRenderMode();
	materialResource->m_customShader = data.getCustomShader().resource();
	materialResource->m_samplers = data.getSamplers();

	for (const auto& [name, uniform] : data.getUniforms())
	{
		materialResource->m_uniformProperties[name] = uniform.value;
	}
	
}

void MaterialAsset::setName(const std::string& name)
{
	data.name = name;
}

std::string MaterialAsset::getName() const
{
	return data.name;
}

void MaterialAsset::setCustomShader(ShaderAssetRef& customShader)
{
	data.setCustomShader(customShader);
}

ShaderAssetRef MaterialAsset::getCustomShader() const
{
	return data.getCustomShader();
}

void MaterialAsset::setMaterialRenderMode(MaterialRenderMode renderMode)
{
	data.setMaterialRenderMode(renderMode);;
}

MaterialRenderMode MaterialAsset::getMaterialRenderMode() const
{
	return data.getMaterialRenderMode();
}

void MaterialAsset::setSampler(const std::string& name, std::shared_ptr<TextureSampler> sampler)
{
	data.setSampler(name, sampler);
}

std::shared_ptr<TextureSampler> MaterialAsset::getSampler(const std::string& name)
{
	const auto& samplers = data.getSamplers();
	auto it = samplers.find(name);
	return it != samplers.end() ? it->second : nullptr;
}

void MaterialAsset::setSamplerEnabled(const std::string& name, bool isEnabled)
{
	auto sampler = getSampler(name);
	if (sampler)
	{
		sampler->isActive = isEnabled;
	}
}

void MaterialAsset::setUniformValue(const std::string& name, const Value& v)
{
	data.setUniform(name, v);
}

Value MaterialAsset::getUniformValue(const std::string& name)
{
	auto it = data.getUniforms().find(name);
	return it != data.getUniforms().end() ? it->second.value : Value{};
}

ShaderResourceRef MaterialAsset::getActiveShader() const
{
	if (data.getMaterialRenderMode() != MaterialRenderMode::Custom)
	{
		return MaterialDataParser::getShaderFromRenderMode(data.getMaterialRenderMode());
	}
	else
	{
		return data.getCustomShader().resource();
	}
}

MaterialAssetRef MaterialAsset::clone(bool isEngineOwned) const
{
	AssetBuildDescriptor desc;
	desc.aType = AssetType::MATERIAL;
	desc.name = data.name + "_clone";
	desc.isEngineOwned = isEngineOwned;
	
	MaterialCreateDescriptor materialDesc;
	materialDesc.data = data;
	
	MaterialAssetRef cloned = Engine::get()->getSubSystem<Assets>()->createAsset(desc, materialDesc).as<MaterialAsset>();
	return cloned;
}

std::map<std::string, std::shared_ptr<TextureSampler>> MaterialAsset::getSamplers()
{
	return data.getSamplers();
}

std::map<std::string, EditableUniform> MaterialAsset::getUniformProperties()
{
	return data.getUniforms();
}

void MaterialAsset::serialize(nlohmann::json& j) const
{
	j = nlohmann::json::object();

	std::stringstream ss;
	{
		cereal::JSONOutputArchive archive(ss);
		archive(cereal::make_nvp("data", data));
	}

	auto parsed = nlohmann::json::parse(ss.str(), nullptr, false);

	if (!parsed.is_discarded() && parsed.contains("data"))
		j["data"] = parsed["data"];
	else
		j["data"] = nlohmann::json::object();
}

void MaterialAsset::deserialize(const nlohmann::json& j)
{
	if (!j.contains("data"))
		return;

	nlohmann::json wrapper;
	wrapper["data"] = j["data"];

	std::stringstream ss(wrapper.dump());

	try
	{
		cereal::JSONInputArchive archive(ss);
		archive(cereal::make_nvp("data", data));
	}
	catch (const std::exception& e)
	{
		logError("Failed to deserialize material: {}", e.what());
	}
}

void MaterialAsset::fillBuildDescriptor(ResourceBuildDescriptor& resourceBuildDesc)
{
	auto materialDesc = dynamic_cast<MaterialCreateDescriptor*>(&resourceBuildDesc);
	if (!materialDesc)
	{
		logError("Invalid Descriptor specified.");
		return;
	}

	materialDesc->data = data;
}