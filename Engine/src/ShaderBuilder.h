#pragma once

#include <string>
#include <map>

#include "Core.h"
#include "Shader.h"
#include "Component.h"

class EngineAPI CustomShaderBuilder
{
public:
	enum class ShaderOverride
	{
		PBR,
		Pixel
	};

	static CustomShaderBuilder& create(const std::string& filePath, ShaderOverride shaderOverride);

	ShaderComponent build();

private:
	CustomShaderBuilder(const std::string& filePath, ShaderOverride shaderOverride);

private:
	std::string m_filepath;
};