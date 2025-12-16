#include "render/Material.h"

#include "core/Logger.h"
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
#include "render/ShadersInfo.h"

#include <filesystem>

namespace {
	struct MaterialManagerRegistration {
		MaterialManagerRegistration() {
			AssetFactory::registerManager(AssetType::MATERIAL, std::make_shared<MaterialAssetManager>());
		}
	} _materialManagerRegistration;
}

bool MaterialAssetManager::copyFiles(const std::string& fileLocation, AssetInfo& aInfo)
{
	return false;
}

ResourceWrapper<ResourceBase> MaterialAssetManager::load(AssetInfo& aInfo)
{
	std::ifstream is(aInfo.fullFilePath);
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

	return ResourceWrapper<ResourceBase>::empty;
}

void MaterialAssetManager::save(const AssetWrapper<ResourceBase>& mat, const AssetInfo& aInfo)
{
	auto projectDir = Engine::get()->getProjectDirectory();
	std::ofstream os(aInfo.fullFilePath);
	cereal::JSONOutputArchive oarchive(os);

	try
	{
		oarchive(*mat.as<Material>().resource().get());
	}
	catch (const cereal::Exception& e)
	{
		logError("Serialization Error occured: {}", e.what());
	}
}

//static AssetFnRegister<AssetType::MATERIAL> assetRegister(AssetTraits<Material>::load);

Material::Material()
{}

void Material::use()
{
	int slot = 0;
	auto shader = m_shader;
	for (const auto& [name, sampler] : m_samplers)
	{
		// if texture is empty use dummy texture
		AssetWrapper<Texture> texture;
		if (!sampler || sampler->texture.isEmpty())
		{
			texture = BuiltInAssets::getByName<Texture>(SGE_TEXTURE_WHITE); // maybe use disgusting pink texture?
		}
		else
		{
			texture = sampler->texture;
		}

		texture.get()->setSlot(slot);
		texture.get()->bind();

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

		slot++;
	}

	for (const auto& [name, property] : m_uniformProperties)
	{
		shader->setUniformValue(name, property.value);
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
	auto it = m_samplers.find(name);
	if (it == m_samplers.end())
	{
		logError("Sampler '{}' not found in Material.", name);
		throw std::runtime_error("");
	}

	return it->second;
}

void Material::setSampler(const std::string& name, std::shared_ptr<TextureSampler> sampler)
{
	m_samplers[name] = sampler;
}

void Material::setSamplerEnabled(const std::string& name, bool isEnabled)
{
	m_samplers[name]->isActive = isEnabled;
}

AssetWrapper<Material> Material::import(const std::string& fileLocation, MaterialImportSettings desc)
{
	desc.aType = AssetType::MATERIAL;
	desc.origFilePath = fileLocation;
	return Engine::get()->getSubSystem<Assets>()->importAsset(fileLocation, desc).as<Material>();
}

ResourceWrapper<Material> Material::create(MaterialRenderMode renderMode)
{
	auto mat = Factory<Material>::create();
	mat->setMaterialRenderMode(renderMode);
	return mat;
}

ResourceWrapper<Material> Material::create(MaterialRenderMode renderMode, const AssetWrapper<Shader>& customShader)
{
	auto mat = Factory<Material>::create();
	mat->setMaterialRenderMode(renderMode, customShader);
	return mat;
}

void Material::updateAsset(const AssetWrapper<Material>& material, AssetUpdateDescriptor desc)
{
	Engine::get()->getSubSystem<Assets>()->updateAsset(material, desc);
}

ResourceWrapper<Material> Material::clone(bool isTransient) const
{
	auto newMaterial = Material::create(m_renderMode);

	newMaterial->m_samplers = m_samplers;
	newMaterial->m_shader = m_shader;
	newMaterial->m_uniformProperties = m_uniformProperties;

	return newMaterial;
}

//bool Material::isOpaque() const
//{
//	auto it = m_uniformProperties.find(SHADER_PROPERTY_PBR_OPACITY_FACTOR);
//	if (it != m_uniformProperties.end()) {
//		float opacity = std::get<float>(it->second);  // throws if wrong type
//		return opacity == 1.f;
//	}
//	return true;
//}

bool parseEditablePragmaLine(const std::string& line, Material::EditableUniform& editableUniform) {
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

void Material::parseUniforms(const std::string& sourceCode)
{
	m_uniformProperties.clear();
	m_samplers.clear();

	std::istringstream stream(sourceCode);
	std::string line;

	auto& uniformProperties = m_uniformProperties;

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
				m_samplers[name] = std::make_shared<TextureSampler>();
			}

			
			isNextLineEditableUniform = false;
		}
	}
}

void Material::setShader(AssetWrapper<Shader> shader)
{
	m_shader = shader;

	std::string sourceCode;
	ShadersInfo sInfo = shader.resource()->getShadersInfo();
	sourceCode += sInfo.vertexCode + "\n";
	sourceCode += sInfo.fragmentCode + "\n";
	sourceCode += sInfo.computeCode + "\n";
	sourceCode += sInfo.geometryCode + "\n";
	sourceCode += sInfo.tessControlCode + "\n";
	sourceCode += sInfo.tessEvaluationCode + "\n";

	parseUniforms(sourceCode);
}

void Material::update()
{
	auto oldUniforms = m_uniformProperties;
	auto oldSamplers = m_samplers;

	setShader(m_shader);

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
		m_shader->setUniformValue(name, uniform.value);
	}

	// TODO fix
	//if (!projectionTexture.resource().isEmpty())
	//{
	//	setProjectionTexture(projectionTexture);
	//}
}

void Material::setUniformValue(const std::string& name, const Value& v)
{
	m_uniformProperties[name].value = v;
}

void Material::setName(const std::string& name)
{
	m_name = name;
}
std::string Material::getName() const
{
	return m_name;
}

void Material::setMaterialRenderMode(MaterialRenderMode renderMode, const AssetWrapper<Shader>& customShader)
{
	m_renderMode = renderMode;

	if (m_renderMode == MaterialRenderMode::Opaque)
	{
		setShader(BuiltInAssets::getByName<Shader>(SGE_SHADER_DEFFERED_PBR_GEOM));
	}
	else if (m_renderMode == MaterialRenderMode::Transparent)
	{
		setShader(BuiltInAssets::getByName<Shader>(SGE_SHADER_FORWARD_PBR));
	}
	else if (m_renderMode == MaterialRenderMode::Terrain)
	{
		setShader(BuiltInAssets::getByName<Shader>(SGE_SHADER_TERRAIN));
	}
	else if (m_renderMode == MaterialRenderMode::Custom)
	{
		if (customShader.isEmpty())
		{
			logError("Specified render mode is custom, therefore you must assign a valid shader as argument.");
			return;
		}
		setShader(customShader);
	}
}

MaterialRenderMode Material::getMaterialRenderMode() const
{
	return m_renderMode;
}
