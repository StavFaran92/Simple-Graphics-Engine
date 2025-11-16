#pragma once

#include <filesystem>
#include "core/Core.h"

class EngineAPI Path
{
public:
	Path() = default;
	Path(std::filesystem::path path, std::filesystem::path root);
	virtual ~Path() = default;


	std::filesystem::path raw() const;

	std::filesystem::path absolute() const;

	void setPath(std::filesystem::path path);

protected: 
	std::filesystem::path m_path;
	std::filesystem::path m_root;


};