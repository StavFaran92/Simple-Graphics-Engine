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
	Volume,
	Custom,

	// This must be last
	None,
};

struct MaterialData
{
	std::string name;
	ResourceWrapper<Shader> shader;
	//MaterialRenderMode m_renderMode = MaterialRenderMode::None;
	std::map<std::string, std::shared_ptr<TextureSampler>> samplers;
	std::map<std::string, ResourceWrapper<Texture>> textures;
	std::map<std::string, Value> uniformProperties;
};

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


struct MaterialLoadDescriptor : public ResourceLoadDescriptor
{
	MaterialLoadDescriptor();

	MaterialRenderMode renderMode;
	MaterialData data;
};

//Resource
class EngineAPI Material : public Resource
{
public:

	Material();
	~Material() = default;

	static ResourceWrapper<Material> create(MaterialData data);

	static ResourceWrapper<Material> load(const std::string& fileLocation, MaterialLoadDescriptor desc = {});

	void use();
	void release();

	void setSampler(const std::string& name, std::shared_ptr<TextureSampler> sampler);
	std::shared_ptr<TextureSampler> getSampler(const std::string& name);
	void setSamplerEnabled(const std::string& name, bool isEnabled);

	void setUniformValue(const std::string& name, const Value& v);

private:
	MaterialData m_data;
};

// Asset
class EngineAPI MaterialAsset : public Asset
{
public:
	using ResourceType = Material;

	using Asset::Asset;

	void setMaterialRenderMode(MaterialRenderMode renderMode);
	MaterialRenderMode getMaterialRenderMode() const;

	void setCustomShader(AssetHandle<ShaderAsset>& customShader);
	AssetHandle<ShaderAsset> getCustomShader() const;

	void setUniformValue(const std::string& name, const Value& v);

	ResourceWrapper<Shader> getActiveShader() const;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(m_name);
		SERIALIZED_MEMBER(m_customShader);
		SERIALIZED_MEMBER(m_renderMode);
		SERIALIZED_MEMBER(m_samplers);
		SERIALIZED_MEMBER(m_uniformProperties);
	}

private:
	void update();

	void parseUniforms(const std::string& sourceCode);

	void parseFromShader(ResourceWrapper<Shader> shader);

	static ResourceWrapper<Shader> getShaderFromRenderMode(MaterialRenderMode renderMode);

	std::string m_name;
	AssetHandle<ShaderAsset> m_customShader;
	MaterialRenderMode m_renderMode = MaterialRenderMode::None;
	std::map<std::string, std::shared_ptr<TextureSampler>> m_samplers;
	std::map<std::string, EditableUniform> m_uniformProperties;

protected:
	void fillData(ResourceLoadDescriptor& loadDesc) const override;
};