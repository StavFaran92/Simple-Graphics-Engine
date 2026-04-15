#pragma once

#include "core/Core.h"
#include "memory/Asset.h"
#include "memory/AssetAliases.h"

struct EngineAPI LuaScriptCreateDescriptor : public ResourceBuildDescriptor
{
	std::string script;
};

struct EngineAPI LuaScriptLoadDescriptor : public ResourceLoadDescriptor
{
};

// Resource
class EngineAPI LuaScript : public Resource
{
public:
	static LuaScriptResourceRef load(const std::string& fileLocation, LuaScriptLoadDescriptor desc = {});

	static LuaScriptResourceRef create();

	std::string filepath;
};

// Asset
class EngineAPI LuaScriptAsset : public Asset
{
public:
	using ResourceType = LuaScript;

	using Asset::Asset;

	void serialize(nlohmann::json& j) const override;
	void deserialize(const nlohmann::json& j) override;

	std::string filepath;
};
