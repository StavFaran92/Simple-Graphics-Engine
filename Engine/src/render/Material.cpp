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
#include "memory/AssetRef.h"
#include "render/MaterialDataParser.h"
#include "render/TerrainLayer.h"
#include "texture/Texture.h"
#include "texture/TextureSampler.h"
#include "debug/RenderDocDebugHelper.h"
#include "texture/TextureTransformer.h"

void useSamplerInShader(const std::string& name, const TextureSampler& sampler, ShaderResourceRef& shader, int slot)
{
	// if texture is empty use dummy texture
	TextureResourceRef texture = sampler.texture;
	texture->setSlot(slot);
	texture->bind();

	// set sampler2D (e.g. material.diffuse3 to the currently active texture unit)
	shader->setUniformValue(name + ".texture", slot);

	shader->setUniformValue(name + ".isActive", sampler.state.isActive);
	shader->setUniformValue(name + ".xOffset", sampler.state.xOffset);
	shader->setUniformValue(name + ".yOffset", sampler.state.yOffset);
	shader->setUniformValue(name + ".xScale", sampler.state.xScale);
	shader->setUniformValue(name + ".yScale", sampler.state.yScale);
	shader->setUniformValue(name + ".channelMaskR", sampler.state.channelMaskR);
	shader->setUniformValue(name + ".channelMaskG", sampler.state.channelCount > 1 ? sampler.state.channelMaskG : 0);
	shader->setUniformValue(name + ".channelMaskB", sampler.state.channelCount > 2 ? sampler.state.channelMaskB : 0);
	shader->setUniformValue(name + ".channelMaskA", sampler.state.channelCount > 3 ? sampler.state.channelMaskA : 0);

	//shader->setUniformValue(name + ".channelMaskR", sampler ? sampler->state.channelMaskR : 1);
	//shader->setUniformValue(name + ".channelMaskG", sampler ? (sampler->state.channelCount > 1 ? sampler->state.channelMaskG : 0) : 0);
	//shader->setUniformValue(name + ".channelMaskB", sampler ? (sampler->state.channelCount > 2 ? sampler->state.channelMaskB : 0) : 0);
	//shader->setUniformValue(name + ".channelMaskA", sampler ? (sampler->state.channelCount > 3 ? sampler->state.channelMaskA : 0) : 0);
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

	int slot = 7;

	// Set samplers
	for (const auto& [name, sampler] : m_samplers)
	{
		useSamplerInShader(name, sampler, shader, slot);
		slot++;
	}

	// Set uniforms
	for (const auto& [name, value] : m_uniforms)
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

TextureSampler Material::getSampler(const std::string& name)
{
	auto it = m_samplers.find(name);
	return it != m_samplers.end() ? it->second : TextureSampler{};
}

void Material::setSampler(const std::string& name, const TextureSampler& sampler)
{
	m_samplers[name] = sampler;
}

Value Material::getUniformValue(const std::string& name)
{
	auto it = m_uniforms.find(name);
	return it != m_uniforms.end() ? it->second : Value{};
}

MaterialResourceRef Material::create(MaterialRenderMode renderMode)
{
	auto mat = Factory<Material>::create();
	mat->m_renderMode = renderMode;
	return mat;
}

//MaterialResourceRef Material::clone(bool isEngineOwned) const
//{
//	auto newMaterial = Material::create(m_renderMode);
//
//	newMaterial->m_samplers = m_samplers;
//	newMaterial->m_uniformProperties = m_uniformProperties;
//	newMaterial->m_customShader = m_customShader;
//	newMaterial->m_name = m_name;
//
//	return newMaterial;
//}

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
	m_uniforms[name] = v;
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
	
	auto sampler = std::make_shared<TextureSamplerAsset>(samplerChannels);
	sampler->state.isActive = true;
	sampler->texture = textureAsset;
	setProperty(shaderPropertyName, sampler);
}

void MaterialAsset::fillData(ResourceRef<Resource> resource) const
{
	auto materialResource = resource.as<Material>();
	materialResource->m_name = data.name;
	materialResource->m_renderMode = data.getMaterialRenderMode();
	materialResource->m_customShader = data.getCustomShader().resource();

	for (const auto& [name, spec] :data.getLayout().getAllProperties())
	{
		Value value = getProperty(name);

		if (spec.type == MaterialPropertyType::SAMPLER)
		{
			auto sampler = std::get<std::shared_ptr<TextureSamplerAsset>>(value);
			TextureSampler textureSamplerResource;
			textureSamplerResource.texture = sampler->texture.resource();
			textureSamplerResource.state = sampler->state;
			materialResource->m_samplers[name] = textureSamplerResource;

		}
		else
		{
			materialResource->m_uniforms[name] = value;
		}


	}

	if (data.getMaterialRenderMode() == MaterialRenderMode::Terrain)
	{
		//RenderDocDebugHelper::startFrameCapture();

		auto& whiteTexture = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_WHITE).resource();
		auto& blackTexture = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_BLACK).resource();
		auto& checkerboarcTexture = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_CHECKERBOARD).resource();

		int layerIndex = 0;
		for (auto& [layerName, value] : data.getAllProptiesOfType(MaterialPropertyType::TERRAIN_LAYER))
		{
			auto layer = std::get<std::shared_ptr<TerrainLayerAsset>>(value);

			// Set Texture Pack 0
			auto albedo = layer->albedoTexture.isEmpty() ? whiteTexture : layer->albedoTexture.resource();
			auto normal = layer->normalTexture.isEmpty() ? whiteTexture : layer->normalTexture.resource();
			
			TextureResourceRef texturePack0 = TextureTransformer::packTextures(
				albedo, 0,
				albedo, 1,
				albedo, 2,
				normal, 0);

			{
				TextureSampler textureSamplerResource;
				textureSamplerResource.texture = texturePack0;
				textureSamplerResource.state.isActive = true;
				textureSamplerResource.state.channelCount = 4;
				auto samplerName = layerName + ".texturePack0";
				materialResource->m_samplers[samplerName] = textureSamplerResource;
			}

			// Set Texture Pack 1
			auto metalness = layer->metallicTexture.isEmpty() ? whiteTexture : layer->metallicTexture.resource();
			auto roughness = layer->roughnessTexture.isEmpty() ? whiteTexture : layer->roughnessTexture.resource();
			auto ao = layer->aoTexture.isEmpty() ? whiteTexture : layer->aoTexture.resource();

			TextureResourceRef texturePack1 = TextureTransformer::packTextures(
				metalness, 0,
				roughness, 0,
				ao, 0,
				normal, 1);

			{
				TextureSampler textureSamplerResource;
				textureSamplerResource.texture = texturePack1;
				textureSamplerResource.state.isActive = true;
				textureSamplerResource.state.channelCount = 4;
				auto samplerName = layerName + ".texturePack1";
				materialResource->m_samplers[samplerName] = textureSamplerResource;
			}

			// Set Opacity mask
			{
				TextureSampler textureSamplerResource;

				// TODO remove
				if (layerIndex == 0)
				{
					textureSamplerResource.texture = whiteTexture;
				}
				else if (layerIndex == 1)
				{
					textureSamplerResource.texture = checkerboarcTexture;
				}
				else
				{
					textureSamplerResource.texture = layer->mask.isEmpty() ? blackTexture : layer->mask.resource();
				}
				
				textureSamplerResource.state.isActive = true;
				textureSamplerResource.state.channelCount = 1;
				auto samplerName = layerName + ".opacityMask";
				materialResource->m_samplers[samplerName] = textureSamplerResource;
			}

			layerIndex++;
		}

		

		//RenderDocDebugHelper::stopFrameCapture();

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

const MaterialLayout& MaterialAsset::getLayout() const
{
	return data.getLayout();
}

void MaterialAsset::setProperty(const std::string& name, const Value& v)
{
	data.setProperty(name, v);
}

Value MaterialAsset::getProperty(const std::string& name) const
{
	return data.getProperty(name);
}

std::unordered_map<std::string, Value> MaterialAsset::getAllProperties() const
{
	return data.getAllProperties();
}

std::unordered_map<std::string, Value> MaterialAsset::getAllProptiesOfType(MaterialPropertyType type) const
{
	return data.getAllProptiesOfType(type);
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

std::vector<AssetRef<Asset>> MaterialAsset::gatherDependencies() const
{
	std::vector<AssetRef<Asset>> dependencies;
	auto samplers = getAllProptiesOfType(MaterialPropertyType::SAMPLER);
	for (const auto& [sName, _] : samplers)
	{
		auto sampler = getProperty<std::shared_ptr<TextureSamplerAsset>>(sName);
		dependencies.push_back(sampler->texture);
	}
	return dependencies;
}
