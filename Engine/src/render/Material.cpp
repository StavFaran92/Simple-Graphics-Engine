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
{

}

void Material::use(ResourceWrapper<Shader> externalShader)
{
	ResourceWrapper<Shader> shader;
	if (externalShader.isEmpty())
	{
		shader = m_shader.resource();
	}
	if (shader.isEmpty())
	{
		logError("Invalid shader for material. ");
		return;
	}

	int slot = 0;
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
		shader->setUniformValue("material." + name + ".texture", slot);
		shader->setUniformValue("material." + name + ".xOffset", sampler->xOffset);
		shader->setUniformValue("material." + name + ".yOffset", sampler->yOffset);
		shader->setUniformValue("material." + name + ".xScale", sampler->xScale);
		shader->setUniformValue("material." + name + ".yScale", sampler->yScale);
		shader->setUniformValue("material." + name + ".channelMaskR", sampler->channelMaskR);
		shader->setUniformValue("material." + name + ".channelMaskG", sampler->channelCount > 1 ? sampler->channelMaskG : 0);
		shader->setUniformValue("material." + name + ".channelMaskB", sampler->channelCount > 2 ? sampler->channelMaskB : 0);
		shader->setUniformValue("material." + name + ".channelMaskA", sampler->channelCount > 3 ? sampler->channelMaskA : 0);

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

ResourceWrapper<Material> Material::create()
{

	auto mat = Factory<Material>::create();

	//mat->m_samplers[Texture::TextureType::Albedo] = std::make_shared<TextureSampler>(3);
	//mat->m_samplers[Texture::TextureType::Normal] = std::make_shared<TextureSampler>(3);
	//mat->m_samplers[Texture::TextureType::Metallic] = std::make_shared<TextureSampler>(1);
	//mat->m_samplers[Texture::TextureType::Roughness] = std::make_shared<TextureSampler>(1);
	//mat->m_samplers[Texture::TextureType::AmbientOcclusion] = std::make_shared<TextureSampler>(1);

	return mat;
}

void Material::updateAsset(const AssetWrapper<Material>& material, AssetUpdateDescriptor desc)
{
	Engine::get()->getSubSystem<Assets>()->updateAsset(material, desc);
}

ResourceWrapper<Material> Material::clone(bool isTransient) const
{
	auto newMaterial = Material::create();

	newMaterial->m_samplers = m_samplers;
	newMaterial->m_shader = m_shader;
	newMaterial->m_uniformProperties = m_uniformProperties;

	return newMaterial;
}

bool Material::isOpaque() const
{
	return 1; // todo fix
	//return opacityFactor == 1;
}

void Material::parseUniforms(const std::string& sourceCode)
{
	m_uniformProperties.clear();
	m_samplers.clear();

	std::regex uniformRegex(R"(uniform\s+(\w+)\s+(\w+)\s*;)");
	std::smatch match;
	std::string::const_iterator searchStart(sourceCode.cbegin());

	auto& uniformProperties = m_uniformProperties;

	while (std::regex_search(searchStart, sourceCode.cend(), match, uniformRegex)) {
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
		else if (type == "sampler2D") {
			m_samplers[name] = std::make_shared<TextureSampler>();
		}

		searchStart = match.suffix().first;
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