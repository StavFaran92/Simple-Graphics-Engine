#pragma once

#include "core/Core.h"
#include "serialize/CerealHelpers.h"
#include "render/Shader.h"
#include "texture/TextureSampler.h"

#include <map>
#include <string>

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


class EngineAPI MaterialData
{
public:
	std::string name;

	void setMaterialRenderMode(MaterialRenderMode renderMode);

	MaterialRenderMode getMaterialRenderMode() const;

	void setCustomShader(ShaderAssetRef& customShader);

	ShaderAssetRef getCustomShader() const;

	const std::map<std::string, EditableUniform>& getUniforms() const;

	bool setUniform(const std::string& name, const Value& value);

	const std::map<std::string, std::shared_ptr<TextureSampler>>& getSamplers() const;

	void setSampler(const std::string& name, std::shared_ptr<TextureSampler> sampler);

	bool isParsed() const;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(name);
		SERIALIZED_MEMBER(m_renderMode);
		SERIALIZED_MEMBER(m_customShader);
		SERIALIZED_MEMBER(m_uniforms);
		SERIALIZED_MEMBER(m_samplers);
	}

private:
	friend class MaterialDataParser;

	void update();

	bool m_isParsed = false;
	MaterialRenderMode m_renderMode = MaterialRenderMode::Opaque;
	ShaderAssetRef m_customShader;
	std::map<std::string, EditableUniform> m_uniforms;
	std::map<std::string, std::shared_ptr<TextureSampler>> m_samplers;
};