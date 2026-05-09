#pragma once

#include <string>
#include "core/Configurations.h"
#include "memory/ResourceBase.h"
#include "fileSystem/ScopedPath.h"

enum class AssetVisibility {
	Public,   // shows up in asset picker/browser
	Internal  // hidden from user selection
};

struct ResourceLoadDescriptor
{
	virtual ~ResourceLoadDescriptor() = default;

	std::string sourcePath;
};

struct ResourceBuildDescriptor
{
	virtual ~ResourceBuildDescriptor() = default;
};

struct AssetBuildDescriptor
{
	std::string name;
	AssetType aType = AssetType::NONE;
	bool isEngineOwned = false;
	AssetVisibility visibility = AssetVisibility::Internal;
	ScopedPath targetDirectory; // the user authored destination directory
	std::string assetDirectory; // nested directory, relative to scope
};

struct AssetUpdateDescriptor
{
	std::string name;
	ScopedPath targetDirectory; // the user authored updated destination directory
};