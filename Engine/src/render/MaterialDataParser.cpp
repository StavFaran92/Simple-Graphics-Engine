#include "render/MaterialDataParser.h"

#include "memory/BuiltInResources.h"

#include <regex>

bool MaterialDataParser::parseEditablePragmaLine(const std::string& line, EditableUniform& editableUniform) {
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

std::vector<float> MaterialDataParser::parseFloatTuple(const std::string& s) {
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

void MaterialDataParser::parseUniforms(const std::string& sourceCode, MaterialData& data)
{
	data.m_uniforms.clear();
	data.m_samplers.clear();

	std::istringstream stream(sourceCode);
	std::string line;

	auto& uniformProperties = data.m_uniforms;
	auto& samplers = data.m_samplers;

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

	for (const auto& [name, uniform] : data.m_uniforms)
	{
		data.m_uniforms[name].value = uniform.value;
	}
}

void MaterialDataParser::parseFromShader(ResourceWrapper<Shader> shader, MaterialData& data)
{
	std::string sourceCode;
	ShadersInfo sInfo = shader->getShadersInfo();
	sourceCode += sInfo.vertexCode + "\n";
	sourceCode += sInfo.fragmentCode + "\n";
	sourceCode += sInfo.computeCode + "\n";
	sourceCode += sInfo.geometryCode + "\n";
	sourceCode += sInfo.tessControlCode + "\n";
	sourceCode += sInfo.tessEvaluationCode + "\n";

	parseUniforms(sourceCode, data);
}

ResourceWrapper<Shader> MaterialDataParser::getShaderFromRenderMode(MaterialRenderMode renderMode)
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

ResourceWrapper<Shader> MaterialDataParser::getActiveShader(const MaterialData& data)
{
	if (data.m_renderMode != MaterialRenderMode::Custom)
	{
		return getShaderFromRenderMode(data.m_renderMode);
	}
	else
	{
		return data.m_customShader.resource();
	}
}

void MaterialDataParser::parse(MaterialData& data)
{
	auto oldUniforms = data.m_uniforms;
	auto oldSamplers = data.m_samplers;

	ResourceWrapper<Shader> shader = getActiveShader(data);

	if (shader.isEmpty())
	{
		data.m_uniforms.clear();
		data.m_samplers.clear();
		return;
	}

	parseFromShader(shader, data);

	auto& newSamplers = data.m_samplers;
	for (const auto [name, sampler] : oldSamplers)
	{
		auto iter = newSamplers.find(name);
		if (iter != newSamplers.end())
		{
			iter->second = sampler;
		}
	}

	auto& newUniforms = data.m_uniforms;
	for (const auto [name, value] : oldUniforms)
	{
		auto iter = newUniforms.find(name);
		if (iter != newUniforms.end())
		{
			iter->second = value;
		}
	}
}
