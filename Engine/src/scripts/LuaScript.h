#pragma once

#include "core/Core.h"
#include "memory/Asset.h"

struct LuaScriptImportSettings : public AssetCreateDescriptor
{

};

struct LuaScriptAssetManager : public AssetManager
{
	bool copyFiles(const std::string& fileLocation, AssetInfo& aInfo) override;
	ResourceWrapper<ResourceBase> load(AssetInfo& aInfo) override;
	void save(const ResourceWrapper<ResourceBase>& script, const AssetInfo& aInfo) override;
};

class EngineAPI LuaScript : public ResourceBase
{
public:
	static ResourceWrapper<LuaScript> import(const std::string& fileLocation, LuaScriptImportSettings settings = {});
	static ResourceWrapper<LuaScript> create();
	static void updateAsset(const ResourceWrapper<LuaScript>& script, AssetUpdateDescriptor desc);
};