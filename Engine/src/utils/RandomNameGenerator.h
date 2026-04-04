#pragma once

#include <string>
#include "core/Core.h"

class EngineAPI RandomNameGenerator
{
public:
	static std::string generateName();
private:
	static size_t s_nameLength; // tweakable later
};
