#pragma once

#include <string>
#include "core/Configurations.h"
#include "memory/ResourceBase.h"
#include "fileSystem/ScopedPath.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct ResourceLoadDescriptor
{
	virtual ~ResourceLoadDescriptor() = default;

	virtual ResourceWrapper<Resource> loadResource() = 0;

	virtual nlohmann::json fillParams() const { return {}; } // todo consider remove

	std::string sourcePath;
};

struct ResourceBuildDescriptor
{
	virtual ~ResourceBuildDescriptor() = default;

	virtual ResourceWrapper<Resource> createResource() = 0;
};

struct AssetBuildDescriptor
{
	std::string name;
	AssetType aType = AssetType::NONE;
	bool isEngineOwned = false;
	ScopedPath targetDirectory; // the user authored destination directory
	std::string assetDirectory; // nested directory, relative to scope
};

struct AssetUpdateDescriptor
{
	std::string name;
	ScopedPath targetDirectory; // the user authored updated destination directory
};