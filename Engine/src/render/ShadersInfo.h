#pragma once

#include <string>

struct ShadersInfo
{
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::string computeCode;
	std::string tessControlCode;
	std::string tessEvaluationCode;
};