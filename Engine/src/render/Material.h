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

	Resource<Material> clone() const;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(m_samplers);
	}

	static Resource<Material> import(const std::string& fileLocation, const MaterialImportSettings& settings = {});
	static Resource<Material> loadTransient(const std::string& fileLocation, const MaterialImportSettings& settings = {});
	static Resource<Material> create();

protected:
	void setTexturesInShader(Resource<Shader>& shader);
	void setTextureInShader(Resource<Shader>& shader, Texture::TextureType ttype, int slot);

protected:
	std::map<Texture::TextureType, std::shared_ptr<TextureSampler>> m_samplers;

	std::string m_name;
};
