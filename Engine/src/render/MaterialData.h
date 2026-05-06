#pragma once

#include "core/Core.h"
#include "serialize/CerealHelpers.h"
#include "render/Shader.h"
#include "texture/TextureSampler.h"

#include "MaterialProperty.h"

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

class EngineAPI MaterialData
{
public:
	std::string name;

	void setMaterialRenderMode(MaterialRenderMode renderMode);
	MaterialRenderMode getMaterialRenderMode() const;

	void setCustomShader(ShaderAssetRef& customShader);
	ShaderAssetRef getCustomShader() const;

	MaterialLayout& getLayout();
	const MaterialLayout& getLayout() const;

	void setProperty(const std::string& name, const Value& v);
	Value getProperty(const std::string& name) const;
	std::unordered_map<std::string, Value> getAllProperties() const;
	std::unordered_map<std::string, Value> getAllProptiesOfType(MaterialPropertyType type) const;

	bool isParsed() const;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(name);
		SERIALIZED_MEMBER(m_renderMode);
		SERIALIZED_MEMBER(m_customShader);
		SERIALIZED_MEMBER(m_layout);
		SERIALIZED_MEMBER(m_properties);
	}

private:
	friend class MaterialDataParser;

	void update();

	bool m_isParsed = false;
	MaterialRenderMode m_renderMode = MaterialRenderMode::Opaque;
	ShaderAssetRef m_customShader;
	MaterialLayout m_layout;
	std::unordered_map<std::string, Value> m_properties;
};