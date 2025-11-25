#pragma once

#include <filesystem>
#include "core/Core.h"

class EngineAPI ScopedPath
{
public:
	enum class Type { None, Content, Engine };


	ScopedPath();
	static ScopedPath ContentPath(std::filesystem::path path = "");
	static ScopedPath EnginePath(std::filesystem::path path = "");

	std::filesystem::path relative() const;
	std::filesystem::path absolute() const;
	std::filesystem::path scoped() const;

	ScopedPath::Type type() const;

	void setPath(std::filesystem::path path);

private: 
	ScopedPath(std::filesystem::path path, std::filesystem::path root);

	std::filesystem::path m_path;
	std::filesystem::path m_root;
	Type m_type = Type::None;
};