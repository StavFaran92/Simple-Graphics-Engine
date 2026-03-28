#pragma once

#include "core/Core.h"
#include "render/Material.h"

class EngineAPI MaterialDataParser
{
public:
	static void parse(MaterialData& data);

	static ResourceWrapper<Shader> getShaderFromRenderMode(MaterialRenderMode renderMode);

private:
	static bool parseEditablePragmaLine(const std::string& line, EditableUniform& editableUniform);

	static std::vector<float> parseFloatTuple(const std::string& s);

	static void parseUniforms(const std::string& sourceCode, MaterialData& data);

	static void parseFromShader(ResourceWrapper<Shader> shader, MaterialData& data);

	static ResourceWrapper<Shader> getActiveShader(const MaterialData& data);

};