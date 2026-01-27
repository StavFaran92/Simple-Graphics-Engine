#pragma once

#include <string>
#include <map>
#include "core/Configurations.h"
#include "memory/ResourceBase.h"
#include "fileSystem/ScopedPath.h"

#include <nlohmann/json.hpp>


using json = nlohmann::json;

struct ResourceLoadDescriptor
{
	virtual ~ResourceLoadDescriptor() = default;

	virtual ResourceWrapper<Resource> loadResource() = 0;

	virtual nlohmann::json fillParams() const { return {}; }

	std::string sourcePath;
};

struct AssetCreateDescriptor
{
	std::string name;
	AssetType aType = AssetType::NONE;
	std::map<std::string, std::string> engineAttributes;
	bool isEngineOwned = false;
	bool isTransient = false;
	bool isCompositeAsset = false; // this asset is composed of multiple external files 
	ScopedPath targetDirectory;

	ResourceLoadDescriptor* resourceDescriptor = nullptr;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(AssetCreateDescriptor,
		name,
		aType,
		engineAttributes,
		isEngineOwned,
		isTransient,
		isCompositeAsset
		);

	//virtual nlohmann::json fillParams() const { return {}; }

	//timestamp
	//size
};

struct AssetUpdateDescriptor
{
	std::string assetDirectory;
	std::map<std::string, std::string> attributes;
	std::string name;
};