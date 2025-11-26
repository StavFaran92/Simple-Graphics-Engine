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

#include <filesystem>

namespace {
	struct MaterialManagerRegistration {
		MaterialManagerRegistration() {
			AssetFactory::registerManager(AssetType::MATERIAL, std::make_shared<MaterialAssetManager>());
		}
	} _materialManagerRegistration;
}

bool MaterialAssetManager::copyFiles(const std::string& fileLocation, AssetInfo& aInfo)
{
	return false;
}

ResourceWrapper<ResourceBase> MaterialAssetManager::load(AssetInfo& aInfo)
{
	std::ifstream is(aInfo.fullFilePath);
	cereal::JSONInputArchive iarchive(is);
	ResourceWrapper<Material> material = Factory<Material>::create();

	try
	{
		iarchive(*material.get());
		return material;

	}
	catch (const cereal::Exception& e)
	{
		logError("Deserialization Error occured: {}", e.what());
	}

	return ResourceWrapper<ResourceBase>::empty;
}

void MaterialAssetManager::save(const AssetWrapper<ResourceBase>& mat, const AssetInfo& aInfo)
{
	auto projectDir = Engine::get()->getProjectDirectory();
	std::ofstream os(aInfo.fullFilePath);
	cereal::JSONOutputArchive oarchive(os);

	try
	{
		oarchive(*mat.as<Material>().resource().get());
	}
	catch (const cereal::Exception& e)
	{
		logError("Serialization Error occured: {}", e.what());
	}
}

//static AssetFnRegister<AssetType::MATERIAL> assetRegister(AssetTraits<Material>::load);

Material::Material()
{}

void Material::use()
{
	int slot = 0;
	auto shader = m_shader.resource();
	for (const auto& [name, sampler] : m_samplers)
	{
		// if texture is empty use dummy texture
		AssetWrapper<Texture> texture = sampler->texture;
		if (sampler->texture.resource().isEmpty())
		{
			texture = BuiltInAssets::getByName<Texture>(SGE_TEXTURE_WHITE);
		}

		texture.get()->setSlot(slot);
		texture.get()->bind();

		

		// set sampler2D (e.g. material.diffuse3 to the currently active texture unit)
		shader->setUniformValue(name + ".texture", slot);
		shader->setUniformValue(name + ".xOffset", sampler->xOffset);
		shader->setUniformValue(name + ".yOffset", sampler->yOffset);
		shader->setUniformValue(name + ".xScale", sampler->xScale);
		shader->setUniformValue(name + ".yScale", sampler->yScale);
		shader->setUniformValue(name + ".channelMaskR", sampler->channelMaskR);
		shader->setUniformValue(name + ".channelMaskG", sampler->channelCount > 1 ? sampler->channelMaskG : 0);
		shader->setUniformValue(name + ".channelMaskB", sampler->channelCount > 2 ? sampler->channelMaskB : 0);
		shader->setUniformValue(name + ".channelMaskA", sampler->channelCount > 3 ? sampler->channelMaskA : 0);

		slot++;
	}

	for (const auto& [name, property] : m_uniformProperties)
	{
		shader->setUniformValue(name, property);
	}


	//m_shader.get()->setUniformValue("material.roughnessFactor", roughnessFactor);
	//m_shader.get()->setUniformValue("material.metallicFactor", metallicFactor);
	//m_shader.get()->setUniformValue("material.colorDiffuse", colorDiffuse);
	//m_shader.get()->setUniformValue("material.opacityFactor", opacityFactor);
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
	if (it == m_samplers.end())
	{
		logError("Sampler '{}' not found in Material.", name);
		throw std::runtime_error("");
	}

	return it->second;
}

void Material::setSampler(const std::string& name, std::shared_ptr<TextureSampler> sampler)
{
	m_samplers[name] = sampler;
}

AssetWrapper<Material> Material::import(const std::string& fileLocation, MaterialImportSettings desc)
{
	desc.aType = AssetType::MATERIAL;
	desc.origFilePath = fileLocation;
	return Engine::get()->getSubSystem<Assets>()->importAsset(fileLocation, desc).as<Material>();
}

ResourceWrapper<Material> Material::create(MaterialRenderMode renderMode)
{
	auto mat = Factory<Material>::create();
	mat->setMaterialRenderMode(renderMode);
	return mat;
}

void Material::updateAsset(const AssetWrapper<Material>& material, AssetUpdateDescriptor desc)
{
	Engine::get()->getSubSystem<Assets>()->updateAsset(material, desc);
}

ResourceWrapper<Material> Material::clone(bool isTransient) const
{
	auto newMaterial = Material::create(m_renderMode);

	newMaterial->m_samplers = m_samplers;
	newMaterial->m_shader = m_shader;
	newMaterial->m_uniformProperties = m_uniformProperties;

	return newMaterial;
}

bool Material::isOpaque() const
{
	auto it = m_uniformProperties.find(SHADER_PROPERTY_PBR_OPACITY_FACTOR);
	if (it != m_uniformProperties.end()) {
		float opacity = std::get<float>(it->second);  // throws if wrong type
		return opacity == 1.f;
	}
	return true;
}

void Material::parseUniforms(const std::string& sourceCode)
{
	m_uniformProperties.clear();
	m_samplers.clear();

	std::istringstream stream(sourceCode);
	std::string line;
	bool nextUniformIsEditable = false;

	auto& uniformProperties = m_uniformProperties;

	std::regex uniformRegex(R"(uniform\s+(\w+)\s+(\w+)\s*;)");

	while (std::getline(stream, line)) {
		// Trim whitespace
		line.erase(0, line.find_first_not_of(" \t"));

		// Check for pragma
		if (line.find("#pragma editable") == 0) {
			nextUniformIsEditable = true;
			continue;
		}

		// Match uniform declaration
		std::smatch match;
		if (std::regex_search(line, match, uniformRegex)) {
			if (!nextUniformIsEditable) continue; // skip if not marked editable
			nextUniformIsEditable = false; // reset after one use

			std::string type = match[1].str();
			std::string name = match[2].str();

			if (type == "float") {
				uniformProperties[name] = 0.0f;
			}
			else if (type == "vec2") {
				uniformProperties[name] = glm::vec2(0.0f);
			}
			else if (type == "vec3") {
				uniformProperties[name] = glm::vec3(0.0f);
			}
			else if (type == "vec4") {
				uniformProperties[name] = glm::vec4(0.0f);
			}
			else if (type == "int") {
				uniformProperties[name] = 0;
			}
			else if (type == "uint") {
				uniformProperties[name] = 0u;
			}
			else if (type == "mat3") {
				uniformProperties[name] = glm::mat3(1.0f);
			}
			else if (type == "mat4") {
				uniformProperties[name] = glm::mat4(1.0f);
			}
			else if (type == "PBR_Sampler") {
				m_samplers[name] = std::make_shared<TextureSampler>();
			}
		}
	}
}

void Material::setShader(AssetWrapper<Shader> shader)
{
	m_shader = shader;

	const std::string& sourceCode = shader.resource()->getSourceCode();
	parseUniforms(sourceCode);
}

void Material::update()
{
	auto oldUniforms = m_uniformProperties;
	auto oldSamplers = m_samplers;

	parseUniforms(m_shader.resource()->getSourceCode());


	auto& newSamplers = m_samplers;
	for (const auto [name, sampler] : oldSamplers)
	{
		auto iter = newSamplers.find(name);
		if (iter != newSamplers.end())
		{
			iter->second = sampler;
		}
	}

	auto& newUniforms = m_uniformProperties;
	for (const auto [name, value] : oldUniforms)
	{
		auto iter = newUniforms.find(name);
		if (iter != newUniforms.end())
		{
			iter->second = value;
		}
	}

	for (const auto& [name, value] : m_uniformProperties)
	{
		m_shader.resource()->setUniformValue(name, value);
	}

	// TODO fix
	//if (!projectionTexture.resource().isEmpty())
	//{
	//	setProjectionTexture(projectionTexture);
	//}
}

void Material::setUniformValue(const std::string& name, const Value& v)
{
	m_uniformProperties[name] = v;
}

void Material::setName(const std::string& name)
{
	m_name = name;
}
std::string Material::getName() const
{
	return m_name;
}

void Material::setMaterialRenderMode(MaterialRenderMode renderMode)
{
	m_renderMode = renderMode;

	if (m_renderMode == MaterialRenderMode::Opaque)
	{
		setShader(BuiltInAssets::getByName<Shader>(SGE_SHADER_DEFFERED_PBR_GEOM));
	}
	else if (m_renderMode == MaterialRenderMode::Transparent)
	{
		setShader(BuiltInAssets::getByName<Shader>(SGE_SHADER_FORWARD_PBR));
	}
}

MaterialRenderMode Material::getMaterialRenderMode() const
{
	return m_renderMode;
}
