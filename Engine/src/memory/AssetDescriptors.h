#pragma once

#include <string>
#include <map>
#include "core/Configurations.h"
#include "memory/ResourceBase.h"
#include "fileSystem/ScopedPath.h"

#include <nlohmann/json.hpp>


using json = nlohmann::json;

using CreateFunc = std::function<ResourceWrapper<Resource>()>;

struct ResourceLoadDescriptor
{
	

	virtual ~ResourceLoadDescriptor() = default;

	//virtual ResourceWrapper<Resource> loadResource() = 0;

	virtual nlohmann::json fillParams() const { return {}; }

	virtual ResourceWrapper<Resource> loadResource() = 0;

	std::string sourcePath;
};

struct AssetCreateDescriptor
{
	std::string name;
	std::string sourcePath;
	AssetType aType = AssetType::NONE;
	std::map<std::string, std::string> engineAttributes;
	bool isEngineOwned = false;
	bool isTransient = false;
	bool isCompositeAsset = false; // this asset is composed of multiple external files 
	ScopedPath targetDirectory;
	std::string assetDirectory; // todo consider remove

	ResourceLoadDescriptor* resourceDescriptor = nullptr;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(AssetCreateDescriptor,
		name,
		aType,
		engineAttributes,
		isEngineOwned,
		isTransient,
		isCompositeAsset
		);

	template<typename T, typename... Args>
	T* makeResourceDescriptor(Args&&... args)
	{
		static_assert(std::is_base_of_v<ResourceLoadDescriptor, T>);
		T* obj = new T(std::forward<Args>(args)...);
		resourceDescriptor = obj;
		return obj;
	}

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