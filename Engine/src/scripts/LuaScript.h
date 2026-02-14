#pragma once

#include "core/Core.h"
#include "memory/Asset.h"

// Resource
class EngineAPI LuaScript : public Resource
{
public:
	struct LoadDescriptor : public ResourceLoadDescriptor
	{
		ResourceWrapper<Resource> loadResource() override {
			return LuaScript::load(sourcePath, *this);
		}
	};

	static ResourceWrapper<LuaScript> load(const std::string& fileLocation, LoadDescriptor desc = {});

	static ResourceWrapper<LuaScript> create();

	std::string filepath;
};

// Asset
class EngineAPI LuaScriptAsset : public Asset
{
public:
	using ResourceType = LuaScript;

	using Asset::Asset;
};