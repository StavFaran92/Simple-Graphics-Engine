#pragma once

#include <string>
#include "fileSystem/Path.h"
#include "fileSystem/FileSystem.h"
#include "fileSystem/ContentPath.h"

#include "core/Core.h"

#include <filesystem>

class EngineAPI WorkingDirectory
{
public:
	WorkingDirectory();
	ContentPath path() const;

	void back();

	void enterFolder(const std::string& folder);

	std::string string() const;

	std::filesystem::directory_iterator iter() const;
public:
	//Path m_path;

	ContentPath m_path;
};