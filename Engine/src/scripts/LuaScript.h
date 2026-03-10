#pragma once

#include "core/Core.h"
#include "memory/Asset.h"

struct EngineAPI LuaScriptCreateDescriptor : public ResourceCreateDescriptor
{
	//MaterialData data;

	ResourceWrapper<Resource> createResource() override;
};

struct EngineAPI LuaScriptLoadDescriptor : public ResourceLoadDescriptor
{
	ResourceWrapper<Resource> loadResource() override;
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