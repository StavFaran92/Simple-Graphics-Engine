#pragma once

#include <string>
#include <memory>

#include "render/IShaderParser.h"
#include "render/ShadersInfo.h"

class ShaderLoader
{
public:
	struct LoadParams
	{
		bool extendShader = false;
	};

	ShaderLoader(std::shared_ptr<IShaderParser> shaderParser, LoadParams eParams);

	std::string readShader(const std::string& path);

	inline LoadParams getLoadParams() { return m_params; };
	inline LoadParams setLoadParams(LoadParams eParams) { m_params = eParams; };

	bool parseGLSLShader(const std::string& sCode, ShadersInfo& shader);

private:
	std::shared_ptr<IShaderParser> m_shaderParser = nullptr;

	LoadParams m_params;
};