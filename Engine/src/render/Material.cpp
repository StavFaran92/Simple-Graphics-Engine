#include "render/Material.h"

#include "core/Logger.h"
#include <functional>

MaterialLoadDescriptor::MaterialLoadDescriptor()
{
	createFunc = std::bind(&Material::load, sourcePath, *this);
}
#include <GL\glew.h>

#include "core/Engine.h"
#include "runtime/Context.h"

#include "memory/AssetLoader.h"
#include "core/Factory.h"
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/archives/json.hpp>
#include <iostream>
#include <fstream>
#include <regex>
#include "memory/BuiltInAssets.h"
#include "memory/BuiltInResources.h"
#include "render/ShadersInfo.h"

#include <filesystem>

void MaterialAsset::fillData(ResourceLoadDescriptor& loadDesc) const
{
	auto& mat = static_cast<MaterialLoadDescriptor&>(loadDesc);
	mat.data.name = m_name;
	mat.data.samplers = m_samplers;
	mat.data.shader = m_customShader.resource();
	for (auto& [name, uniform] : m_uniformProperties)
	{
		mat.data.uniformProperties[name] = uniform.value;
	}
}

//static AssetFnRegister<AssetType::MATERIAL> assetRegister(AssetTraits<Material>::load);

Material::Material()
{}

void useSamplerInShader(const std::string& name, std::shared_ptr<TextureSampler> sampler, ResourceWrapper<Shader>& shader, int slot)
{
	// if texture is empty use dummy texture
	AssetHandle<TextureAsset> texture;
	if (!sampler || sampler->texture.isEmpty())
	{
		texture = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_WHITE); // maybe use disgusting pink texture?
	}
	else
	{
		texture = sampler->texture;
	}

	texture.resource()->setSlot(slot);
	texture.resource()->bind();

	// set sampler2D (e.g. material.diffuse3 to the currently active texture unit)
	shader->setUniformValue(name + ".texture", slot);

	shader->setUniformValue(name + ".isActive", sampler->isActive);
	shader->setUniformValue(name + ".xOffset", sampler ? sampler->xOffset : 0.0f);
	shader->setUniformValue(name + ".yOffset", sampler ? sampler->yOffset : 0.0f);
	shader->setUniformValue(name + ".xScale", sampler ? sampler->xScale : 1.0f);
	shader->setUniformValue(name + ".yScale", sampler ? sampler->yScale : 1.0f);
	shader->setUniformValue(name + ".channelMaskR", sampler ? sampler->channelMaskR : 1);
	shader->setUniformValue(name + ".channelMaskG", sampler ? (sampler->channelCount > 1 ? sampler->channelMaskG : 0) : 0);
	shader->setUniformValue(name + ".channelMaskB", sampler ? (sampler->channelCount > 2 ? sampler->channelMaskB : 0) : 0);
	shader->setUniformValue(name + ".channelMaskA", sampler ? (sampler->channelCount > 3 ? sampler->channelMaskA : 0) : 0);
}

void Material::use()
{
	ResourceWrapper<Shader> shader = m_data.shader;

	if (shader.isEmpty())
	{
		logWarning("Material {} shader is invalid.", m_data.name);
		return;
	}

	shader->use();

	int slot = 0;

	// Set samplers
	for (const auto& [name, sampler] : m_data.samplers)
	{
		useSamplerInShader(name, sampler, shader, slot);
		slot++;
	}

	// Set Textures
	for (const auto& [name, texture] : m_data.textures)
	{
		texture.get()->setSlot(slot);
		texture.get()->bind();
		shader->setUniformValue(name, slot);
		slot++;
	}

	// Set uniforms
	for (const auto& [name, value] : m_data.uniformProperties)
	{
		shader->setUniformValue(name, value);
	}
}

void Material::release()
{
	for (auto i = 0; i < 3; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

std::shared_ptr<TextureSampler> Material::getSampler(const std::string& name)
{
	return getSampler(name);
}

void Material::setSampler(const std::string& name, std::shared_ptr<TextureSampler> sampler)
{
	setSampler(name, sampler);
}

void Material::setSamplerEnabled(const std::string& name, bool isEnabled)
{
	getSampler(name)->isActive = isEnabled;
}

ResourceWrapper<Material> Material::create(MaterialData data)
{
	auto mat = Factory<Material>::create();
	mat->m_data = data;
	return mat;
}

//ResourceWrapper<Material> Material::clone(bool isTransient) const
//{
//	auto newMaterial = Material::create(m_renderMode);
//
//	newMaterial->m_persistentBlock = m_persistentBlock;
//	newMaterial->m_nonPersistentBlock = m_nonPersistentBlock;
//	newMaterial->m_customShader = m_customShader;
//
//	return newMaterial;
//}

ResourceWrapper<Shader> MaterialAsset::getActiveShader() const
{
	if (m_renderMode != MaterialRenderMode::Custom)
	{
		return getShaderFromRenderMode(m_renderMode);
	}
	else
	{
		return m_customShader.resource();
	}
}

ResourceWrapper<Shader> MaterialAsset::getShaderFromRenderMode(MaterialRenderMode renderMode)
{
	switch(renderMode)
	{
	case MaterialRenderMode::Opaque:
		return BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_DEFFERED_PBR_GEOM);
	case MaterialRenderMode::Transparent:
		return BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_FORWARD_PBR);
	case MaterialRenderMode::Terrain:
		return BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_TERRAIN);
	case MaterialRenderMode::Volume:
		return BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_VOLUME);
	}

	return ResourceWrapper<Shader>::empty;
}

bool parseEditablePragmaLine(const std::string& line, EditableUniform& editableUniform) {
	// Check if line contains #pragma editable
	std::regex pragmaRegex(R"(^\s*#pragma\s+editable)");

	if (!std::regex_search(line, pragmaRegex)) {
		logWarning("Invalid pragma editable line: {}", line);
		return false;
	}

	// Parse default: capture content inside parentheses OR a scalar value
	std::regex defaultRegex(R"(default\s*=\s*(?:\(([^)]*)\)|([0-9.+-eEfF]+)))");
	std::regex minRegex(R"(min\s*=\s*([0-9.+-eEfF]+))");
	std::regex maxRegex(R"(max\s*=\s*([0-9.+-eEfF]+))");

	std::smatch m;

	if (std::regex_search(line, m, defaultRegex)) {
		// m[1] has parenthesized content, m[2] has scalar
		editableUniform.defaultValueRaw = m[1].matched ? m[1].str() : m[2].str();
	}

	if (std::regex_search(line, m, minRegex)) {
		editableUniform.minValue = std::stof(m[1].str());
	}

	if (std::regex_search(line, m, maxRegex)) {
		editableUniform.maxValue = std::stof(m[1].str());
	}

	return true;
}

std::vector<float> parseFloatTuple(const std::string& s) {
	std::vector<float> values;
	std::regex numRegex(R"([0-9.+-]+)");
	std::smatch match;
	std::string copy = s;

	while (std::regex_search(copy, match, numRegex)) {
		values.push_back(std::stof(match[0].str()));
		copy = match.suffix();
	}
	return values;
}

void MaterialAsset::parseUniforms(const std::string& sourceCode)
{
	m_uniformProperties.clear();
	m_samplers.clear();

	std::istringstream stream(sourceCode);
	std::string line;

	auto& uniformProperties = m_uniformProperties;
	auto& samplers = m_samplers;

	std::regex uniformRegex(R"(uniform\s+(\w+)\s+(\w+)\s*;)");
	EditableUniform pendingMeta;
	bool isNextLineEditableUniform = false;

	while (std::getline(stream, line)) {
		// Trim
		line.erase(0, line.find_first_not_of(" \t"));

		if (line.find("#pragma editable") == 0) {
			isNextLineEditableUniform = parseEditablePragmaLine(line, pendingMeta);
			continue;
		}

		std::smatch match;
		if (isNextLineEditableUniform && std::regex_search(line, match, uniformRegex)) {
			std::string type = match[1];
			std::string name = match[2];

			pendingMeta.uniformName = name;
			pendingMeta.type = type;

			

			// Parse default
			std::vector<float> defVals = parseFloatTuple(pendingMeta.defaultValueRaw);

			if (type == "float") 
			{
				pendingMeta.value = defVals.size() > 0 ? defVals[0] : 0.0f;
				uniformProperties[name] = pendingMeta;
			}
			else if (type == "int") 
			{
				pendingMeta.value = defVals.size() > 0 ? static_cast<int>(defVals[0]) : 0;
				uniformProperties[name] = pendingMeta;
			}
			else if (type == "uint") 
			{
				pendingMeta.value = defVals.size() > 0 ? static_cast<unsigned int>(defVals[0]) : 0u;
				uniformProperties[name] = pendingMeta;
			}
			else if (type == "vec2") 
			{
				glm::vec2 val(0.0f);
				for (size_t i = 0; i < std::min<size_t>(2, defVals.size()); ++i)
					val[i] = defVals[i];
				pendingMeta.value = val;
				uniformProperties[name] = pendingMeta;
			}
			else if (type == "vec3") 
			{
				glm::vec3 val(0.0f);
				for (size_t i = 0; i < std::min<size_t>(3, defVals.size()); ++i)
					val[i] = defVals[i];
				pendingMeta.value = val;
				uniformProperties[name] = pendingMeta;
			}
			else if (type == "vec4") 
			{
				glm::vec4 val(0.0f);
				for (size_t i = 0; i < std::min<size_t>(4, defVals.size()); ++i)
					val[i] = defVals[i];
				pendingMeta.value = val;
				uniformProperties[name] = pendingMeta;
			}
			else if (type == "mat3") 
			{
				pendingMeta.value = glm::mat3(1.0f);
				uniformProperties[name] = pendingMeta;
			}
			else if (type == "mat4") 
			{
				pendingMeta.value = glm::mat4(1.0f);
				uniformProperties[name] = pendingMeta;
			}
			else if (type == "PBR_Sampler") 
			{
				samplers[name] = std::make_shared<TextureSampler>();
			}

			
			isNextLineEditableUniform = false;
		}
	}
}

void MaterialAsset::parseFromShader(ResourceWrapper<Shader> shader)
{
	std::string sourceCode;
	ShadersInfo sInfo = shader->getShadersInfo();
	sourceCode += sInfo.vertexCode + "\n";
	sourceCode += sInfo.fragmentCode + "\n";
	sourceCode += sInfo.computeCode + "\n";
	sourceCode += sInfo.geometryCode + "\n";
	sourceCode += sInfo.tessControlCode + "\n";
	sourceCode += sInfo.tessEvaluationCode + "\n";

	parseUniforms(sourceCode);
}

void MaterialAsset::update()
{
	auto oldUniforms = m_uniformProperties;
	auto oldSamplers = m_samplers;

	ResourceWrapper<Shader> shader = getActiveShader();

	if (shader.isEmpty())
		return;

	parseFromShader(shader);

	auto& newSamplers = m_samplers;
	for (const auto [name, sampler] : oldSamplers)
	{
		auto iter = newSamplers.find(name);
		if (iter != newSamplers.end())
		{
			iter->second = sampler;
		}
	}

	auto& newUniforms = m_uniformProperties;
	for (const auto [name, value] : oldUniforms)
	{
		auto iter = newUniforms.find(name);
		if (iter != newUniforms.end())
		{
			iter->second = value;
		}
	}

	for (const auto& [name, uniform] : m_uniformProperties)
	{
		shader->setUniformValue(name, uniform.value);
	}

	// TODO fix
	//if (!projectionTexture.resource().isEmpty())
	//{
	//	setProjectionTexture(projectionTexture);
	//}
}

void Material::setUniformValue(const std::string& name, const Value& v)
{
	m_data.uniformProperties[name] = v;
}

void MaterialAsset::setMaterialRenderMode(MaterialRenderMode renderMode)
{
	m_renderMode = renderMode;
	update();
}

void MaterialAsset::setCustomShader(AssetHandle<ShaderAsset>& customShader)
{
	m_customShader = customShader;
	update();
}

AssetHandle<ShaderAsset> MaterialAsset::getCustomShader() const
{
	return m_customShader;
}

MaterialRenderMode MaterialAsset::getMaterialRenderMode() const
{
	return m_renderMode;
}

ResourceWrapper<Material> Material::load(const std::string& fileLocation, MaterialLoadDescriptor desc)
{
	desc.sourcePath = fileLocation;
	std::string filepath = desc.sourcePath;
	auto projectDir = Engine::get()->getProjectDirectory();
	filepath = projectDir + filepath;
	std::ifstream is(filepath);
	cereal::JSONInputArchive iarchive(is);
	ResourceWrapper<Material> material = Factory<Material>::create();

	try
	{
		iarchive(*material.get());
		return material;

	}
	catch (const cereal::Exception& e)
	{
		logError("Deserialization Error occured: {}", e.what());
	}

	return ResourceWrapper<Material>::empty;
}
