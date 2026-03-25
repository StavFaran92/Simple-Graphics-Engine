#pragma once

#include "core/Core.h"
#include "memory/Asset.h"

struct EngineAPI LuaScriptCreateDescriptor : public ResourceBuildDescriptor
{
	std::string script = R"(-- Auto-generated Lua script

Script = {}

function Script:create()
    -- initialization logic
end

function Script:update(dt)
    -- update logic
end

function Script:destroy()
    -- destroy
end
)";
};

struct EngineAPI LuaScriptLoadDescriptor : public ResourceLoadDescriptor
{
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

	void serialize(nlohmann::json& j) const override;
	void deserialize(const nlohmann::json& j) override;

	std::string filepath;
};