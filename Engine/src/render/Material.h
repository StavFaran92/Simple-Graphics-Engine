#pragma once

#include <memory>
#include <map>
#include "core/Core.h"

#include "memory/Resource.h"
#include "render/Shader.h"
#include "texture/Texture.h"
#include "texture/TextureSampler.h"

struct MaterialImportSettings : public BaseAssetParameters
{

};

class EngineAPI Material : public Asset
{
public:
	Material();
	~Material() = default;

	virtual void use(Resource<Shader>& shader);

	virtual void release();

	/**
	 * Gets the mesh's Textures.
	 *
	 * \param colors
	 */
	std::shared_ptr<TextureSampler> getSampler(Texture::TextureType textureType) const;
	void setSampler(Texture::TextureType textureType, std::shared_ptr<TextureSampler> sampler);

	bool hasTexture(Texture::TextureType textureType) const;

	void setTexture(Texture::TextureType textureType, Resource<Texture> textureHandler);

	void setName(const std::string& name);
	std::string getName() const;

	std::vector<Resource<Texture>> getAllTextures() const;

	Resource<Material> clone(bool isTransient) const;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(m_samplers);
		SERIALIZED_MEMBER(m_name);
		SERIALIZED_MEMBER(colorDiffuse);
		SERIALIZED_MEMBER(roughnessFactor);
		SERIALIZED_MEMBER(metallicFactor);
	}

	static Resource<Material> import(const std::string& fileLocation, const MaterialImportSettings& settings = {});
	static Resource<Material> loadTransient(const std::string& fileLocation, const MaterialImportSettings& settings = {});
	static Resource<Material> create(AssetInfo& aInfo);
	static void save(const Resource<Material>& material);

protected:
	void setTexturesInShader(Resource<Shader>& shader);
	void setTextureInShader(Resource<Shader>& shader, Texture::TextureType ttype, int slot);

public:
	std::string m_name;
	std::map<Texture::TextureType, std::shared_ptr<TextureSampler>> m_samplers;
	glm::vec3 colorDiffuse{1.0f, 1.0f, 1.0f};
	float roughnessFactor = 1.f;
	float metallicFactor = 1.f;
	bool isTransparent = false;
};
