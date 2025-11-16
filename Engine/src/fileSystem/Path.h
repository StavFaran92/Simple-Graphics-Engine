#pragma once

#include <filesystem>
#include "core/Core.h"

class EngineAPI Path
{
public:
	Path(std::filesystem::path path, std::filesystem::path root);
	virtual ~Path() = default;

	std::filesystem::path raw() const;

	void setPath(std::filesystem::path path);

protected: 
	std::filesystem::path m_path;
	const std::filesystem::path m_root;


};