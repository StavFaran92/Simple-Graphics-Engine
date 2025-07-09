#pragma once

#include <string>

class IShaderParser
{
public:
	virtual std::string parse(const std::string& path) = 0;
};