#pragma once

#include <string>
#include "fileSystem/ScopedPath.h"
#include "fileSystem/FileSystem.h"

#include "core/Core.h"

#include <filesystem>

class EngineAPI WorkingDirectory
{
public:
	WorkingDirectory();
	ScopedPath path() const;

	void back();

	void enterFolder(const std::string& folder);

	std::string string() const;

	std::filesystem::directory_iterator iter() const;
public:
	//Path m_path;

	ScopedPath m_path;
};