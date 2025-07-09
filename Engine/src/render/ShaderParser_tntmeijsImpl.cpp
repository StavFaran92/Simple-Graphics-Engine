#include "ShaderParser_tntmeijsImpl.h"

#include "Shadinclude.hpp"

std::string ShaderParser_tntmeijs::parse(const std::string& path)
{
	return Shadinclude::load(path);
}
