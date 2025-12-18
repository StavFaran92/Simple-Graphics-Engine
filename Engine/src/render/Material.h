#pragma once

#include <memory>
#include <map>
#include "core/Core.h"

#include "memory/ResourceWrapper.h"
#include "render/Shader.h"
#include "texture/Texture.h"
#include "texture/TextureSampler.h"
#include "memory/Asset.h"
#include "serialize/CerealHelpers.h"

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
	Terrain,
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

	void setSampler(const std::string& name, std::shared_ptr<TextureSampler> sampler);
	std::shared_ptr<TextureSampler> getSampler(const std::string& name);
	void setSamplerEnabled(const std::string& name, bool isEnabled);

	void setUniformValue(const std::string& name, const Value& v);

	void setName(const std::string& name);
	std::string getName() const;

	void setCustomShader(AssetWrapper<Shader>& customShader);
	AssetWrapper<Shader> getCustomShader() const;

	void setMaterialRenderMode(MaterialRenderMode renderMode);
	MaterialRenderMode getMaterialRenderMode() const;

	ResourceWrapper<Material> clone(bool isEngineOwned) const;

	ResourceWrapper<Shader> getActiveShader() const;
private:
	void update();

	void parseUniforms(const std::string& sourceCode);

	void parseFromShader(ResourceWrapper<Shader> shader);

	void setProjectionTexture(AssetWrapper<Texture> texture);

	static ResourceWrapper<Shader> getShaderFromRenderMode(MaterialRenderMode renderMode);
public:

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(m_name);
		SERIALIZED_MEMBER(m_renderMode);
		SERIALIZED_MEMBER(m_samplers);
		SERIALIZED_MEMBER(m_uniformProperties);
	}

	static AssetWrapper<Material> import(const std::string& fileLocation, MaterialImportSettings settings = {});
	static ResourceWrapper<Material> create(MaterialRenderMode renderMode);
	static void updateAsset(const AssetWrapper<Material>& material, AssetUpdateDescriptor desc);

protected:

public:
	struct EditableUniform {
		std::string uniformName;
		std::string type;
		std::string defaultValueRaw; 
		float minValue = std::numeric_limits<float>::lowest();
		float maxValue = std::numeric_limits<float>::max();
		Value value;

		template <class Archive>
		void serialize(Archive& archive) {
			SERIALIZED_MEMBER(uniformName);
			SERIALIZED_MEMBER(type);
			SERIALIZED_MEMBER(defaultValueRaw);
			SERIALIZED_MEMBER(minValue);
			SERIALIZED_MEMBER(maxValue);
			SERIALIZED_MEMBER(value);
		}
	};

	// This will only be used by forward renderer, ignored by deffered
	std::string m_name;

	std::map<std::string, std::shared_ptr<TextureSampler>> m_samplers;
	std::map<std::string, EditableUniform> m_uniformProperties;

private:
	//ResourceWrapper<Shader> m_shader = ResourceWrapper<Shader>::empty;
	AssetWrapper<Shader> m_customShader;
	MaterialRenderMode m_renderMode = MaterialRenderMode::None;

	//ProjectionType projection = ProjectionType::DefaultProjection;
	//AssetWrapper<Texture> projectionTexture;
	//std::shared_ptr<RenderView> renderViewProjection;

};
