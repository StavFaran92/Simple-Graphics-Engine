#pragma once

#include <memory>
#include <map>
#include "core/Core.h"

#include "memory/ResourceWrapper.h"
#include "render/Shader.h"
#include "texture/Texture.h"
#include "texture/TextureSampler.h"
#include "memory/Asset.h"

struct MaterialImportSettings : public AssetCreateDescriptor
{

};

struct MaterialAssetManager : public AssetManager
{
	bool copyFiles(const std::string& fileLocation, AssetInfo& aInfo) override;
	ResourceWrapper<ResourceBase> load(AssetInfo& aInfo) override;
	void save(const AssetWrapper<ResourceBase>& mat, const AssetInfo& aInfo) override;
};

class EngineAPI Material : public ResourceBase
{
public:
	Material();
	~Material() = default;

	virtual void use(ResourceWrapper<Shader>& shader);

	virtual void release();

	/**
	 * Gets the mesh's Textures.
	 *
	 * \param colors
	 */
	std::shared_ptr<TextureSampler> getSampler(Texture::TextureType textureType) const;
	void setSampler(Texture::TextureType textureType, std::shared_ptr<TextureSampler> sampler);

	bool hasTexture(Texture::TextureType textureType) const;

	void setTexture(Texture::TextureType textureType, AssetWrapper<Texture> textureHandler);

	void setName(const std::string& name);
	std::string getName() const;

	std::vector<AssetWrapper<Texture>> getAllTextures() const;

	ResourceWrapper<Material> clone(bool isEngineOwned) const;

	bool isOpaque() const;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(m_samplers);
		SERIALIZED_MEMBER(m_name);
		SERIALIZED_MEMBER(colorDiffuse);
		SERIALIZED_MEMBER(roughnessFactor);
		SERIALIZED_MEMBER(metallicFactor);
		SERIALIZED_MEMBER(opacityFactor);
	}

	static AssetWrapper<Material> import(const std::string& fileLocation, MaterialImportSettings settings = {});
	static ResourceWrapper<Material> create();
	static void updateAsset(const AssetWrapper<Material>& material, AssetUpdateDescriptor desc);

protected:
	void setTexturesInShader(ResourceWrapper<Shader>& shader);
	void setTextureInShader(ResourceWrapper<Shader>& shader, Texture::TextureType ttype, int slot);

public:
	std::string m_name;
	std::map<Texture::TextureType, std::shared_ptr<TextureSampler>> m_samplers;
	glm::vec3 colorDiffuse{1.0f, 1.0f, 1.0f};
	float roughnessFactor = 1.f;
	float metallicFactor = 1.f;
	float opacityFactor = 1.f;

};
