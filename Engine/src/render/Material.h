#pragma once

#include <memory>
#include <map>
#include "core/Core.h"

#include "memory/ResourceWrapper.h"
#include "render/Shader.h"
#include "texture/Texture.h"
#include "texture/TextureSampler.h"
#include "memory/Asset.h"

static const std::string SHADER_PROPERTY_PBR_COLOR_DIFFUSE = "color";
static const std::string SHADER_PROPERTY_PBR_ROUGHNESS_FACTOR = "roughnessFactor";
static const std::string SHADER_PROPERTY_PBR_METALLIC_FACTOR = "metallicFactor";
static const std::string SHADER_PROPERTY_PBR_OPACITY_FACTOR = "opacityFactor";

static const std::string SHADER_PROPERTY_PBR_SAMPLER_ALBEDO = "samplerAlbedo";
static const std::string SHADER_PROPERTY_PBR_SAMPLER_NORMAL = "samplerNormal";
static const std::string SHADER_PROPERTY_PBR_SAMPLER_METALLIC = "samplerMetallic";
static const std::string SHADER_PROPERTY_PBR_SAMPLER_ROUGHNESS = "samplerRoughness";
static const std::string SHADER_PROPERTY_PBR_SAMPLER_AO = "samplerAO";

enum class MaterialRenderMode : int
{
	Opaque,
	Transparent,
	Skybox,
	Unlit,
	UI,
	Custom,

	// This must be last
	None,
};

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

	void use();
	void release();

	std::shared_ptr<TextureSampler> getSampler(const std::string& name);
	void setSampler(const std::string& name, std::shared_ptr<TextureSampler> sampler);

	void setUniformValue(const std::string& name, const Value& v);

	//bool hasTexture(const std::string& name) const;

	//void setTexture(const std::string& name, AssetWrapper<Texture> textureHandler);

	void setName(const std::string& name);
	std::string getName() const;

	void setMaterialRenderMode(MaterialRenderMode renderMode);
	MaterialRenderMode getMaterialRenderMode() const;

	ResourceWrapper<Material> clone(bool isEngineOwned) const;

	bool isOpaque() const;

	//void addTexture(const std::string& name, AssetWrapper<Texture> texture);

	void setProjectionTexture(AssetWrapper<Texture> texture);

	void update();

	void parseUniforms(const std::string& sourceCode);

	void setShader(AssetWrapper<Shader> shader);

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(m_shader);
		SERIALIZED_MEMBER(m_samplers);
		SERIALIZED_MEMBER(m_uniformProperties);
	}

	static AssetWrapper<Material> import(const std::string& fileLocation, MaterialImportSettings settings = {});
	static ResourceWrapper<Material> create(MaterialRenderMode renderMode);
	static void updateAsset(const AssetWrapper<Material>& material, AssetUpdateDescriptor desc);

protected:

public:
	//std::string m_name;
	//std::map<Texture::TextureType, std::shared_ptr<TextureSampler>> m_samplers;
	//glm::vec3 colorDiffuse{1.0f, 1.0f, 1.0f};
	//float roughnessFactor = 1.f;
	//float metallicFactor = 0.f;
	//float opacityFactor = 1.f;

	//enum ProjectionType : int
	//{
	//	DefaultProjection = 0,
	//	Texture2D = 1
	//};

	// This will only be used by forward renderer, ignored by deffered
	std::string m_name;
	MaterialRenderMode m_renderMode = MaterialRenderMode::None;
	AssetWrapper<Shader> m_shader;
	std::map<std::string, std::shared_ptr<TextureSampler>> m_samplers;
	std::map<std::string, Value> m_uniformProperties;

	//ProjectionType projection = ProjectionType::DefaultProjection;
	//AssetWrapper<Texture> projectionTexture;
	//std::shared_ptr<RenderView> renderViewProjection;

};
