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

struct MaterialData
{
	std::string name;
	MaterialRenderMode renderMode = MaterialRenderMode::None;
	AssetHandle<ShaderAsset> customShader = AssetHandle<ShaderAsset>::empty; //optional

	std::map<std::string, Value> uniforms;
	std::map<std::string, std::shared_ptr<TextureSampler>> samplers;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(renderMode);
		SERIALIZED_MEMBER(customShader);
		SERIALIZED_MEMBER(uniforms);
		SERIALIZED_MEMBER(samplers);
	}
};

struct EngineAPI MaterialCreateDescriptor : public ResourceCreateDescriptor
{
	MaterialData data;

	ResourceWrapper<Resource> createResource() override;
};

struct EngineAPI MaterialLoadDescriptor : public ResourceLoadDescriptor
{
	ResourceWrapper<Resource> loadResource() override;
};

//Resource
class EngineAPI Material : public Resource
{
public:
	Material() = default;
	~Material() = default;

	static ResourceWrapper<Material> create(MaterialRenderMode renderMode);

	static ResourceWrapper<Material> load(const std::string& fileLocation, MaterialLoadDescriptor desc = {});

	void use();
	void release();

	std::string getName() const;
	MaterialRenderMode getRenderMode() const;
	ResourceWrapper<Shader> getActiveShader() const;

	void setTexture(const std::string& name, const ResourceWrapper<Texture>& texture);

	void setSampler(const std::string& name, std::shared_ptr<TextureSampler> sampler);
	std::shared_ptr<TextureSampler> getSampler(const std::string& name);

	void setUniformValue(const std::string& name, const Value& v);
	Value getUniformValue(const std::string& name);

	ResourceWrapper<Material> clone(bool isEngineOwned) const;

private:
	std::string m_name;
	ResourceWrapper<Shader> m_customShader;
	MaterialRenderMode m_renderMode = MaterialRenderMode::None;

	std::map<std::string, std::shared_ptr<TextureSampler>> m_samplers;
	std::map<std::string, Value> m_uniformProperties;
};

// Asset
class EngineAPI MaterialAsset : public Asset
{
public:
	using ResourceType = Material;

	using Asset::Asset;

	void bindDependency(const std::string& slot, UUID dependency) override;

	void setName(const std::string& name);
	std::string getName() const;

	void setCustomShader(AssetHandle<ShaderAsset>& customShader);
	AssetHandle<ShaderAsset> getCustomShader() const;

	void setMaterialRenderMode(MaterialRenderMode renderMode);
	MaterialRenderMode getMaterialRenderMode() const;

	void setSampler(const std::string& name, std::shared_ptr<TextureSampler> sampler);
	std::shared_ptr<TextureSampler> getSampler(const std::string& name);

	std::map<std::string, std::shared_ptr<TextureSampler>> getSamplers();
	std::map<std::string, EditableUniform> getUniformProperties();

	void setSamplerEnabled(const std::string& name, bool isEnabled);

	void setUniformValue(const std::string& name, const Value& v);
	Value getUniformValue(const std::string& name);

	ResourceWrapper<Shader> getActiveShader() const;

	AssetHandle<MaterialAsset> clone(bool isEngineOwned) const;

	MaterialData data;
	std::map<std::string, EditableUniform> m_uniformProperties;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(data);
	}

private:
	void update();

	void parseUniforms(const std::string& sourceCode);

	void parseFromShader(ResourceWrapper<Shader> shader);
};