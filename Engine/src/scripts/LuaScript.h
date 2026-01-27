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
			return LuaScript::load(*this);
		}
	};

	static ResourceWrapper<LuaScript> load(const std::string& fileLocation, LoadDescriptor desc = {});
	static ResourceWrapper<LuaScript> load(LoadDescriptor desc);

	static ResourceWrapper<LuaScript> create();

	std::string filepath;
};

// Asset
class EngineAPI LuaScriptAsset : public Asset
{
public:
	using ResourceType = LuaScript;

	using Asset::Asset;

	static AssetHandle<LuaScriptAsset> import(const std::string& fileLocation, AssetCreateDescriptor desc = {});
	static void update(const AssetHandle<LuaScriptAsset>& script, AssetUpdateDescriptor desc);

	void save(const AssetRecord& aInfo) override;

protected:
	bool copyFiles(const std::string& fileLocation, AssetRecord& aInfo) override;
};