#include "render/Material.h"

#include "core/Logger.h"
#include <GL\glew.h>

#include "core/Engine.h"
#include "runtime/Context.h"
#include "systems/CommonTextures.h"
#include "memory/AssetLoader.h"
#include "core/Factory.h"
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/archives/json.hpp>
#include <iostream>
#include <fstream>

#include <filesystem>

template<>
struct AssetTraits<Material>
{
	static bool copyFiles(const std::string& fileLocation, AssetInfo& aInfo)
	{
		return false;
	}

	static void convertAssetLoadParamsToAssetInfo(const std::string& fileLocation, const BaseAssetParameters& params, AssetInfo& aInfo)
	{
		aInfo.aType = AssetType::MATERIAL;
		aInfo.fileName = aInfo.name + ".asset";;
	}

	static Resource<Material> load(AssetInfo& aInfo)
	{
		auto projectDir = Engine::get()->getProjectDirectory();
		std::ifstream is(projectDir + "/" + aInfo.filePath);
		cereal::JSONInputArchive iarchive(is);
		Material* loadedMaterial = new Material();

		try
		{
			iarchive(*loadedMaterial);
			Engine::get()->getMemoryPool().add(aInfo.uuid, loadedMaterial);
			return Resource<Material>(aInfo.uuid);

		}
		catch (const cereal::Exception& e)
		{
			logError("Deserialization Error occured: {}", e.what());
		}

		return Resource<Material>::empty;
	}

	static void save(AssetInfo& aInfo, const Resource<Material>& mat)
	{
		auto projectDir = Engine::get()->getProjectDirectory();
		std::ofstream os(projectDir + "/" + aInfo.filePath);
		cereal::JSONOutputArchive oarchive(os);

		try
		{
			oarchive(*mat.get());
		}
		catch (const cereal::Exception& e)
		{
			logError("Serialization Error occured: {}", e.what());
		}
	}
};

static AssetFnRegister<AssetType::MATERIAL> assetRegister(AssetTraits<Material>::load);

Material::Material()
{
	m_samplers[Texture::TextureType::Albedo] = std::make_shared<TextureSampler>(3);
	m_samplers[Texture::TextureType::Normal] = std::make_shared<TextureSampler>(3);
	m_samplers[Texture::TextureType::Metallic] = std::make_shared<TextureSampler>(1);
	m_samplers[Texture::TextureType::Roughness] = std::make_shared<TextureSampler>(1);
	m_samplers[Texture::TextureType::AmbientOcclusion] = std::make_shared<TextureSampler>(1);
}

void Material::use(Resource<Shader>& shader)
{
	setTexturesInShader(shader);

	shader.get()->setUniformValue("material.roughnessFactor", roughnessFactor);
	shader.get()->setUniformValue("material.metallicFactor", metallicFactor);
	shader.get()->setUniformValue("material.colorDiffuse", colorDiffuse);
	shader.get()->setUniformValue("material.opacityFactor", opacityFactor);
}

void Material::release()
{
	for (auto i = 0; i < 3; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

std::shared_ptr<TextureSampler> Material::getSampler(Texture::TextureType textureType) const
{
	return m_samplers.at(textureType);
}

void Material::setSampler(Texture::TextureType textureType, std::shared_ptr<TextureSampler> sampler)
{
	m_samplers[textureType] = sampler;
}

bool Material::hasTexture(Texture::TextureType textureType) const
{
	auto iter = m_samplers.find(textureType);
	return iter != m_samplers.end() && iter->second->texture.get();
}

void Material::setTextureInShader(Resource<Shader>& shader, Texture::TextureType ttype, int slot)
{
	auto sampler = getSampler(ttype);

	// Activate texture unit i
	glActiveTexture(GL_TEXTURE0 + slot);

	// if texture is empty use dummy texture
	Resource<Texture>& texture = sampler->texture;
	if (sampler->texture.isEmpty())
	{
		texture = Engine::get()->getCommonTextures()->getTexture(CommonTextures::TextureType::WHITE_1X1);
	}

	// Binds iterated texture to target GL_TEXTURE_2D on texture unit i
	glBindTexture(GL_TEXTURE_2D, texture.get()->getID());

	// set sampler2D (e.g. material.diffuse3 to the currently active texture unit)
	shader->setUniformValue("material." + Texture::textureTypeToString(ttype) + ".texture", slot);
	shader->setUniformValue("material." + Texture::textureTypeToString(ttype) + ".xOffset", sampler->xOffset);
	shader->setUniformValue("material." + Texture::textureTypeToString(ttype) + ".yOffset", sampler->yOffset);
	shader->setUniformValue("material." + Texture::textureTypeToString(ttype) + ".xScale", sampler->xScale);
	shader->setUniformValue("material." + Texture::textureTypeToString(ttype) + ".yScale", sampler->yScale);
	shader->setUniformValue("material." + Texture::textureTypeToString(ttype) + ".channelMaskR", sampler->channelMaskR);
	shader->setUniformValue("material." + Texture::textureTypeToString(ttype) + ".channelMaskG", sampler->channelCount > 1 ? sampler->channelMaskG : 0);
	shader->setUniformValue("material." + Texture::textureTypeToString(ttype) + ".channelMaskB", sampler->channelCount > 2 ? sampler->channelMaskB : 0);
	shader->setUniformValue("material." + Texture::textureTypeToString(ttype) + ".channelMaskA", sampler->channelCount > 3 ? sampler->channelMaskA : 0);
}

Resource<Material> Material::import(const std::string& fileLocation, const MaterialImportSettings& settings)
{
	return AssetLoader<Material>::import(fileLocation, settings);
}

Resource<Material> Material::loadTransient(const std::string& fileLocation, const MaterialImportSettings& settings)
{
	return AssetLoader<Material>::loadTransient(fileLocation, settings);
}

Resource<Material> Material::create(AssetInfo& aInfo)
{
	aInfo.aType = AssetType::MATERIAL;
	aInfo.ext = ".asset";

	return AssetLoader<Material>::create(aInfo);
}

void Material::save(const Resource<Material>& material)
{
	AssetLoader<Material>::save(material.get()->m_assetInfo, material);
}

void Material::setTexturesInShader(Resource<Shader>& shader)
{
	// It either has diffuse or albedo
	//setTextureInShader(shader, Texture::Type::Diffuse, 0);
	setTextureInShader(shader, Texture::TextureType::Albedo, 0);
	setTextureInShader(shader, Texture::TextureType::Normal, 1);
	setTextureInShader(shader, Texture::TextureType::Metallic, 2);
	setTextureInShader(shader, Texture::TextureType::Roughness, 3);
	setTextureInShader(shader, Texture::TextureType::AmbientOcclusion, 4);
}

void Material::setTexture(Texture::TextureType textureType, Resource<Texture> textureHandler)
{
	auto iter = m_samplers.find(textureType);
	if (iter == m_samplers.end())
	{
		m_samplers[textureType] = std::make_shared<TextureSampler>();
	}
	m_samplers[textureType]->texture = textureHandler;
}

void Material::setName(const std::string& name)
{
	m_name = name;
}

std::string Material::getName() const
{
	return m_name;
}

std::vector<Resource<Texture>> Material::getAllTextures() const
{
	auto& res = std::vector<Resource<Texture>>();
	for (auto& [_, sampler] : m_samplers)
	{
		res.push_back(sampler->texture);
	}
	return res;
}

Resource<Material> Material::clone(bool isTransient) const
{
	AssetInfo clonedAssetInfo;
	clonedAssetInfo.attributes = m_assetInfo.attributes;
	clonedAssetInfo.isTransient = isTransient;
	auto newMaterial = Material::create(clonedAssetInfo); // tODO rethink this

	for (const auto& sampler : m_samplers)
	{
		newMaterial->setSampler(sampler.first, std::make_shared<TextureSampler>(*sampler.second.get()));
	}

	return newMaterial;
}
