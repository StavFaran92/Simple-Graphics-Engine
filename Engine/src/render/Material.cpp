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
#include "memory/BuiltInResources.h"
#include "render/ShadersInfo.h"
#include "memory/Assets.h"

#include <filesystem>
#include "memory/AssetHandle.h"
#include "texture/Texture.h"
#include "texture/TextureSampler.h"

ResourceWrapper<Shader> getShaderFromRenderMode(MaterialRenderMode renderMode)
{
	switch (renderMode)
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
	ResourceWrapper<Shader> shader = getActiveShader();

	if (shader.isEmpty())
	{
		logWarning("Material {} shader is invalid.", m_name);
		return;
	}

	shader->use();

	int slot = 0;

	// Set samplers
	for (const auto& [name, sampler] : m_samplers)
	{
		useSamplerInShader(name, sampler, shader, slot);
		slot++;
	}

	// Set uniforms
	for (const auto& [name, value] : m_uniformProperties)
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
	auto it = m_samplers.find(name);
	return it != m_samplers.end() ? it->second : nullptr;
}

void Material::setSampler(const std::string& name, std::shared_ptr<TextureSampler> sampler)
{
	m_samplers[name] = sampler;
}

Value Material::getUniformValue(const std::string& name)
{
	auto it = m_uniformProperties.find(name);
	return it != m_uniformProperties.end() ? it->second : Value{};
}

ResourceWrapper<Material> Material::create(MaterialRenderMode renderMode)
{
	auto mat = Factory<Material>::create();
	mat->m_renderMode = renderMode;
	return mat;
}

ResourceWrapper<Material> Material::clone(bool isEngineOwned) const
{
	auto newMaterial = Material::create(m_renderMode);

	newMaterial->m_samplers = m_samplers;
	newMaterial->m_uniformProperties = m_uniformProperties;
	newMaterial->m_customShader = m_customShader;
	newMaterial->m_name = m_name;

	return newMaterial;
}

ResourceWrapper<Shader> Material::getActiveShader() const
{
	if (m_renderMode != MaterialRenderMode::Custom)
	{
		return getShaderFromRenderMode(m_renderMode);
	}
	else
	{
		return m_customShader;
	}
}

void Material::setTexture(const std::string& name, const ResourceWrapper<Texture>& texture)
{
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

void Material::setUniformValue(const std::string& name, const Value& v)
{
	m_uniformProperties[name] = v;
}

std::string Material::getName() const
{
	return m_name;
}

MaterialRenderMode Material::getRenderMode() const
{
	return m_renderMode;
}

ResourceWrapper<Material> Material::load(const std::string& fileLocation, MaterialLoadDescriptor desc)
{
	desc.sourcePath = fileLocation;
	std::string filepath = desc.sourcePath;
	//auto projectDir = Engine::get()->getProjectDirectory();
	//filepath = projectDir + filepath;
	std::ifstream is(filepath);
	cereal::JSONInputArchive iarchive(is);
	MaterialData materialData;
	//ResourceWrapper<Material> material = Factory<Material>::create();

	try
	{
		iarchive(materialData);
		ResourceWrapper<Material> material = Factory<Material>::create();
		material->m_renderMode = materialData.renderMode;
		material->m_customShader = materialData.customShader.resource();
		material->m_name = materialData.name;
		return material;

	}
	catch (const cereal::Exception& e)
	{
		logError("Deserialization Error occured: {}", e.what());
	}

	return ResourceWrapper<Material>::empty;
}

ResourceWrapper<Resource> MaterialCreateDescriptor::createResource()
{
	return Material::create(data.renderMode); //todo fix
}

ResourceWrapper<Resource> MaterialLoadDescriptor::loadResource()
{
	return Material::load(sourcePath, *this);
}

//////////////////////////
// Asset
//////////////////////////

void MaterialAsset::parseUniforms(const std::string& sourceCode)
{
	data.uniforms.clear();
	data.samplers.clear();

	std::istringstream stream(sourceCode);
	std::string line;

	auto& uniformProperties = data.uniforms;
	auto& samplers = data.samplers;

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

	for (const auto& [name, uniform] : data.uniforms)
	{
		data.uniforms[name].value = uniform.value;
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
	auto oldUniforms = data.uniforms;
	auto oldSamplers = data.samplers;

	ResourceWrapper<Shader> shader = getActiveShader();

	if (shader.isEmpty())
		return;

	parseFromShader(shader);

	auto& newSamplers = data.samplers;
	for (const auto [name, sampler] : oldSamplers)
	{
		auto iter = newSamplers.find(name);
		if (iter != newSamplers.end())
		{
			iter->second = sampler;
		}
	}

	auto& newUniforms = data.uniforms;
	for (const auto [name, value] : oldUniforms)
	{
		auto iter = newUniforms.find(name);
		if (iter != newUniforms.end())
		{
			iter->second = value;
		}
	}
}

MaterialAsset::MaterialAsset(const MaterialCreateDescriptor& desc)
{
	data = desc.data;
	update();
}

void MaterialAsset::bindDependency(const std::string& slot, UUID dependency)
{
	// Map slot names to shader property names
	std::string shaderPropertyName;
	int samplerChannels = 1;
	if (slot == "ALBEDO" || slot == SHADER_PROPERTY_PBR_SAMPLER_ALBEDO)
	{
		shaderPropertyName = SHADER_PROPERTY_PBR_SAMPLER_ALBEDO;
		samplerChannels = 3;
	}
	else if (slot == "NORMAL" || slot == SHADER_PROPERTY_PBR_SAMPLER_NORMAL)
	{
		shaderPropertyName = SHADER_PROPERTY_PBR_SAMPLER_NORMAL;
		samplerChannels = 3;
	}
	else if (slot == "ROUGHNESS" || slot == SHADER_PROPERTY_PBR_SAMPLER_ROUGHNESS)
	{
		shaderPropertyName = SHADER_PROPERTY_PBR_SAMPLER_ROUGHNESS;
		samplerChannels = 1;
	}
	else if (slot == "METALLIC" || slot == SHADER_PROPERTY_PBR_SAMPLER_METALLIC)
	{
		shaderPropertyName = SHADER_PROPERTY_PBR_SAMPLER_METALLIC;
		samplerChannels = 1;
	}
	else if (slot == "AO" || slot == SHADER_PROPERTY_PBR_SAMPLER_AO)
	{
		shaderPropertyName = SHADER_PROPERTY_PBR_SAMPLER_AO;
		samplerChannels = 1;
	}
	else
	{
		// Use slot name directly if it doesn't match known patterns
		shaderPropertyName = slot;
	}

	// Cast dependency to TextureAsset
	AssetHandle<TextureAsset> textureAsset(dependency);
	if (textureAsset.isEmpty())
	{
		logWarning("MaterialAsset::bindDependency: Dependency is not a TextureAsset for slot '{}'", slot);
		return;
	}
	
	std::shared_ptr<TextureSampler> sampler = std::make_shared<TextureSampler>(samplerChannels);
	sampler->isActive = true;
	sampler->texture = textureAsset;
	data.samplers[shaderPropertyName] = sampler;
}

void MaterialAsset::fillData(ResourceWrapper<Resource> resource)
{
	auto materialResource = resource.as<Material>();
	materialResource->m_name = data.name;
	materialResource->m_renderMode = data.renderMode;
	materialResource->m_customShader = data.customShader.resource();
	materialResource->m_samplers = data.samplers;

	for (const auto& [name, uniform] : data.uniforms)
	{
		materialResource->m_uniformProperties[name] = uniform.value;
	}
	
}

void MaterialAsset::setName(const std::string& name)
{
	data.name = name;
}

std::string MaterialAsset::getName() const
{
	return data.name;
}

void MaterialAsset::setCustomShader(AssetHandle<ShaderAsset>& customShader)
{
	data.customShader = customShader;
	update();
}

AssetHandle<ShaderAsset> MaterialAsset::getCustomShader() const
{
	return data.customShader;
}

void MaterialAsset::setMaterialRenderMode(MaterialRenderMode renderMode)
{
	data.renderMode = renderMode;
	update();
}

MaterialRenderMode MaterialAsset::getMaterialRenderMode() const
{
	return data.renderMode;
}

void MaterialAsset::setSampler(const std::string& name, std::shared_ptr<TextureSampler> sampler)
{
	data.samplers[name] = sampler;
}

std::shared_ptr<TextureSampler> MaterialAsset::getSampler(const std::string& name)
{
	auto it = data.samplers.find(name);
	return it != data.samplers.end() ? it->second : nullptr;
}

void MaterialAsset::setSamplerEnabled(const std::string& name, bool isEnabled)
{
	auto it = data.samplers.find(name);
	if (it != data.samplers.end() && it->second)
	{
		it->second->isActive = isEnabled;
	}
}

void MaterialAsset::setUniformValue(const std::string& name, const Value& v)
{
	data.uniforms[name].value = v;
}

Value MaterialAsset::getUniformValue(const std::string& name)
{
	auto it = data.uniforms.find(name);
	return it != data.uniforms.end() ? it->second.value : Value{};
}

ResourceWrapper<Shader> MaterialAsset::getActiveShader() const
{
	if (data.renderMode != MaterialRenderMode::Custom)
	{
		return getShaderFromRenderMode(data.renderMode);
	}
	else
	{
		return data.customShader.resource();
	}
}

AssetHandle<MaterialAsset> MaterialAsset::clone(bool isEngineOwned) const
{
	AssetBuildDescriptor desc;
	desc.aType = AssetType::MATERIAL;
	desc.name = data.name + "_clone";
	desc.isEngineOwned = isEngineOwned;
	
	MaterialCreateDescriptor materialDesc;
	materialDesc.data = data;
	
	AssetHandle<MaterialAsset> cloned = Engine::get()->getSubSystem<Assets>()->createAsset(desc, materialDesc).as<MaterialAsset>();
	return cloned;
}

std::map<std::string, std::shared_ptr<TextureSampler>> MaterialAsset::getSamplers()
{
	return data.samplers;
}

std::map<std::string, EditableUniform> MaterialAsset::getUniformProperties()
{
	return data.uniforms;
}

void MaterialAsset::serialize(nlohmann::json& j) const
{
	j = nlohmann::json::object();

	std::stringstream ss;
	{
		cereal::JSONOutputArchive archive(ss);
		archive(cereal::make_nvp("data", data));
	}

	auto parsed = nlohmann::json::parse(ss.str(), nullptr, false);

	if (!parsed.is_discarded() && parsed.contains("data"))
		j["data"] = parsed["data"];
	else
		j["data"] = nlohmann::json::object();
}

void MaterialAsset::deserialize(const nlohmann::json& j)
{
	if (!j.contains("data"))
		return;

	nlohmann::json wrapper;
	wrapper["data"] = j["data"];

	std::stringstream ss(wrapper.dump());

	try
	{
		cereal::JSONInputArchive archive(ss);
		archive(cereal::make_nvp("data", data));
	}
	catch (const std::exception& e)
	{
		logError("Failed to deserialize material: {}", e.what());
	}
}