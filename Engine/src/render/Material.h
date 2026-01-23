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

struct MaterialImportSettings : public AssetCreateDescriptor
{

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

struct MaterialAssetManager : public AssetManager
{
	bool copyFiles(const std::string& fileLocation, AssetRecord& aInfo) override;
	ResourceWrapper<Resource> load(AssetRecord& aInfo) override;
	void save(const AssetHandle<Resource>& mat, const AssetRecord& aInfo) override;
};

class EngineAPI Material : public Resource
{
public:
	struct PersistentBlock
	{
	public:
		std::map<std::string, std::shared_ptr<TextureSampler>>& getSamplers() {
			return m_samplers;
		}

		std::map<std::string, EditableUniform>& getUniformsProperties()
		{
			return m_uniformProperties;
		}

		void setSampler(const std::string& name, std::shared_ptr<TextureSampler> sampler) {
			m_samplers[name] = std::move(sampler);
		}

		std::shared_ptr<TextureSampler> getSampler(const std::string& name) const {
			auto it = m_samplers.find(name);
			return it != m_samplers.end() ? it->second : nullptr;
		}

		void setUniformValue(const std::string& name, const Value& v) {
			m_uniformProperties[name].value = v;
		}

		Value getUniformValue(const std::string& name) const {
			auto it = m_uniformProperties.find(name);
			return it != m_uniformProperties.end() ? it->second.value : Value{};
		}

		template <class Archive>
		void serialize(Archive& archive) {
			SERIALIZED_MEMBER(m_samplers);
			SERIALIZED_MEMBER(m_uniformProperties);
		}

	private:
		friend class Material;
		std::map<std::string, std::shared_ptr<TextureSampler>> m_samplers;
		std::map<std::string, EditableUniform> m_uniformProperties;
	};

	struct NonPersistentBlock
	{
	public:
		void setSampler(const std::string& name, std::shared_ptr<TextureSampler> sampler) {
			m_samplers[name] = std::move(sampler);
		}

		void setUniformValue(const std::string& name, const Value& v) {
			m_uniformProperties[name] = v;
		}

		void setTexture(const std::string& name, const ResourceWrapper<Texture>& texture) {
			m_textures[name] = texture;
		}

	private:
		friend class Material;
		std::map<std::string, std::shared_ptr<TextureSampler>> m_samplers;
		std::map<std::string, ResourceWrapper<Texture>> m_textures;
		std::map<std::string, Value> m_uniformProperties;
	};

	Material();
	~Material() = default;

	void use();
	void release();

	void setSampler(const std::string& name, std::shared_ptr<TextureSampler> sampler);
	std::shared_ptr<TextureSampler> getSampler(const std::string& name);
	void setSamplerEnabled(const std::string& name, bool isEnabled);

	//void setTexture(const std::string& name, const ResourceWrapper<Texture>& texture, int slot);
	void setUniformValue(const std::string& name, const Value& v);

	void setName(const std::string& name);
	std::string getName() const;

	PersistentBlock& getPersistentBlock();
	NonPersistentBlock& getNonPersistentBlock();

	void setCustomShader(AssetHandle<Shader>& customShader);
	AssetHandle<Shader> getCustomShader() const;

	void setMaterialRenderMode(MaterialRenderMode renderMode);
	MaterialRenderMode getMaterialRenderMode() const;

	ResourceWrapper<Material> clone(bool isEngineOwned) const;

	ResourceWrapper<Shader> getActiveShader() const;
private:
	void update();

	void parseUniforms(const std::string& sourceCode);

	void parseFromShader(ResourceWrapper<Shader> shader);

	void setProjectionTexture(AssetHandle<Texture> texture);

	static ResourceWrapper<Shader> getShaderFromRenderMode(MaterialRenderMode renderMode);
public:

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(m_name);
		SERIALIZED_MEMBER(m_renderMode);
		SERIALIZED_MEMBER(m_persistentBlock);
		SERIALIZED_MEMBER(m_customShader);
	}

	static AssetHandle<Material> import(const std::string& fileLocation, MaterialImportSettings settings = {});
	static ResourceWrapper<Material> create(MaterialRenderMode renderMode);
	static void updateAsset(const AssetHandle<Material>& material, AssetUpdateDescriptor desc);

protected:

public:
	std::string m_name;

	//std::map<std::string, std::shared_ptr<TextureSampler>> m_samplers;
	//std::map<std::string, EditableUniform> m_uniformProperties;

	PersistentBlock m_persistentBlock;
	NonPersistentBlock m_nonPersistentBlock;

private:
	AssetHandle<Shader> m_customShader;
	MaterialRenderMode m_renderMode = MaterialRenderMode::None;

	//ProjectionType projection = ProjectionType::DefaultProjection;
	//AssetHandle<Texture> projectionTexture;
	//std::shared_ptr<RenderView> renderViewProjection;

};
