#pragma once

#include "core/Core.h"
#include "render/Material.h"

class EngineAPI MaterialDataParser
{
public:
	static void parse(MaterialData& data);

	static ShaderResourceRef getShaderFromRenderMode(MaterialRenderMode renderMode);

private:
	static bool parseEditablePragmaLine(const std::string& line, PropertySchema& editableUniform);

	static std::vector<float> parseFloatTuple(const std::string& s);

	static void parseUniforms(const std::string& sourceCode, MaterialData& data);

	static void parseFromShader(ShaderResourceRef shader, MaterialData& data);

	static void parseExternalShaderProperties(MaterialRenderMode renderMode, MaterialData& data);

	static ShaderResourceRef getActiveShader(const MaterialData& data);

};