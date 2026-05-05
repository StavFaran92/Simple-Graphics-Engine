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

MaterialLayout& MaterialData::getLayout()
{
	return m_layout;
}

const MaterialLayout& MaterialData::getLayout() const
{
	return m_layout;
}

bool MaterialData::isParsed() const
{
	return m_isParsed;
}

const std::map<std::string, PropertySchema>& MaterialLayout::getAllProperties() const
{
	return m_propertySchemas;
}

void MaterialLayout::addProperty(const std::string& name, const PropertySchema& value)
{
	m_propertySchemas[name] = value;
}

void MaterialLayout::clear()
{
	m_propertySchemas.clear();
}

bool MaterialLayout::hasProperty(const std::string& name) const
{
	return m_propertySchemas.find(name) != m_propertySchemas.end();
}


void MaterialData::setProperty(const std::string& name, const Value& v)
{
	if (getLayout().hasProperty(name))
	{
		// TODO clip to min max
		m_properties[name] = v;
	}
}

Value MaterialData::getProperty(const std::string& name) const
{
	auto iter = m_properties.find(name);
	if (iter != m_properties.end())
		return iter->second;

	// Fall back to schema default so the variant holds the correct type
	auto schemaIter = m_layout.getAllProperties().find(name);
	if (schemaIter != m_layout.getAllProperties().end())
		return schemaIter->second.defaultValue;

	return Value{};
}

std::unordered_map<std::string, Value> MaterialData::getAllProperties() const
{
	std::unordered_map<std::string, Value> result;
	for (auto& [name, _] : m_layout.getAllProperties())
		result[name] = getProperty(name);
	return result;
}

std::unordered_map<std::string, Value> MaterialData::getAllProptiesOfType(MaterialPropertyType type) const
{
	std::unordered_map<std::string, Value> result;
	for (auto& [name, schema] : m_layout.getAllProperties())
	{
		if (schema.type == type)
			result[name] = getProperty(name);
	}
	return result;
}