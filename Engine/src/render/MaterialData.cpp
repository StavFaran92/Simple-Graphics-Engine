#include "render/MaterialData.h"
#include "render/MaterialDataParser.h"

#include "core/Logger.h"

void MaterialData::update()
{
	MaterialDataParser::parse(*this);

	m_isParsed = true;
}

void MaterialData::setMaterialRenderMode(MaterialRenderMode renderMode)
{
	m_renderMode = renderMode;
	update();
}

MaterialRenderMode MaterialData::getMaterialRenderMode() const
{
	return m_renderMode;
}

void MaterialData::setCustomShader(ShaderAssetRef& customShader)
{
	m_customShader = customShader;
	update();
}

ShaderAssetRef MaterialData::getCustomShader() const
{
	return m_customShader;
}

const std::map<std::string, EditableUniform>& MaterialData::getUniforms() const
{
	return m_uniforms;
}

bool MaterialData::setUniform(const std::string& name, const Value& value)
{
	auto it = m_uniforms.find(name);
	if (it == m_uniforms.end())
		return false;

	it->second.value = value;
	return true;
}

const std::map<std::string, std::shared_ptr<TextureSamplerAsset>>& MaterialData::getSamplers() const
{
	return m_samplers;
}

void MaterialData::setSampler(const std::string& name, std::shared_ptr<TextureSamplerAsset> sampler)
{
	auto it = m_samplers.find(name);
	if (it != m_samplers.end())
	{
		it->second = sampler;
	}
	else
	{
		logWarning("Invalid sampler entry: {}", name);
	}
}

bool MaterialData::isParsed() const
{
	return m_isParsed;
}