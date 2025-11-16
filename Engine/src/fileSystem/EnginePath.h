#pragma once

#include "fileSystem/Path.h"

class EnginePath : public Path
{
public:
	EnginePath();
	EnginePath(std::filesystem::path path);
};