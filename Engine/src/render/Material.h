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
#include "render/MaterialData.h"

static const std::string SHADER_PROPERTY_PBR_COLOR_DIFFUSE = "color";
static const std::string SHADER_PROPERTY_PBR_ROUGHNESS_FACTOR = "roughnessFactor";
static const std::string SHADER_PROPERTY_PBR_METALLIC_FACTOR = "metallicFactor";
static const std::string SHADER_PROPERTY_PBR_OPACITY_FACTOR = "opacityFactor";

static const std::string SHADER_PROPERTY_PBR_SAMPLER_ALBEDO = "samplerAlbedo";
static const std::string SHADER_PROPERTY_PBR_SAMPLER_NORMAL = "samplerNormal";
static const std::string SHADER_PROPERTY_PBR_SAMPLER_METALLIC = "samplerMetallic";
static const std::string SHADER_PROPERTY_PBR_SAMPLER_ROUGHNESS = "samplerRoughness";
static const std::string SHADER_PROPERTY_PBR_SAMPLER_AO = "samplerAO";

//The material asset defines what the user chose.
//The material resource defines how to render it efficiently.

struct EngineAPI MaterialCreateDescriptor : public ResourceBuildDescriptor
{
	MaterialData data;
};

struct EngineAPI MaterialLoadDescriptor : public ResourceLoadDescriptor
{
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
	friend class MaterialAsset;

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

	MaterialAsset() = default;
	MaterialAsset(const MaterialCreateDescriptor&);

	void bindDependency(const std::string& slot, UUID dependency) override;

	void fillData(ResourceWrapper<Resource> resource) override;

	void fillBuildDescriptor(ResourceBuildDescriptor& resourceBuildDesc) override;


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

	MaterialData data;

	ResourceWrapper<Shader> getActiveShader() const;

	AssetHandle<MaterialAsset> clone(bool isEngineOwned) const;

	
	//std::map<std::string, EditableUniform> m_uniformProperties;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(data);
	}

	void serialize(nlohmann::json& j) const override;
	void deserialize(const nlohmann::json& j) override;
};