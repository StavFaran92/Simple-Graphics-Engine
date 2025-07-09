#pragma once

#include "render/IShaderParser.h"

class ShaderParser_tntmeijs : public IShaderParser
{
public:
	std::string parse(const std::string& path) override;
};