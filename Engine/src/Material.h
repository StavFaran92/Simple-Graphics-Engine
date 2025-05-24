#pragma once

#include <memory>
#include <map>
#include "Core.h"

#include "Resource.h"
#include "Shader.h"
#include "Texture.h"
#include "TextureSampler.h"

class EngineAPI Material
{
public:
	Material();
	~Material() = default;

	virtual void use(Resource<Shader> shader);

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

	std::shared_ptr<Material> clone() const;

	template <class Archive>
	void serialize(Archive& archive) {
		archive(m_samplers);
	}

protected:
	void setTexturesInShader(Resource<Shader> shader);
	void setTextureInShader(Resource<Shader> shader, Texture::TextureType ttype, int slot);

protected:
	std::map<Texture::TextureType, std::shared_ptr<TextureSampler>> m_samplers;

	std::string m_name;
};
