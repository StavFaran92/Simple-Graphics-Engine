#pragma once

#include <string>
#include <map>

#include "core/Core.h"
#include "render/Shader.h"
#include "component/ShaderComponent.h"

class EngineAPI CustomShaderBuilder
{
public:

	static CustomShaderBuilder& create(const std::string& filePath, ShaderOverride shaderOverride);

	ShaderComponent build();

private:
	CustomShaderBuilder(const std::string& filePath, ShaderOverride shaderOverride);

	void parseUniforms(const std::string& shaderSource, Resource<Shader>& shader);

private:
	std::string m_filepath;
	ShaderOverride m_shaderOverride;
};