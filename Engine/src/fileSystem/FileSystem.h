#pragma once

#include <filesystem>

#include "systems/SubSystem.h"
#include "core/Core.h"

namespace fs = std::filesystem;

class EngineAPI FileSystem : public SubSystem
{
public:
	FileSystem();
	static fs::path toAbsolute(fs::path);
	fs::path getContentDirectory() const;
	fs::path getEngineDirectory() const;
};