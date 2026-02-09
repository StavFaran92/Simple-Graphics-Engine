#pragma once

#include "core/Core.h"
#include "memory/Asset.h"

struct LuaScriptLoadDescriptor : public ResourceLoadDescriptor
{
	LuaScriptLoadDescriptor();
};

// Resource
class EngineAPI LuaScript : public Resource
{
public:
	

	static ResourceWrapper<LuaScript> load(const std::string& fileLocation, LuaScriptLoadDescriptor desc = {});

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