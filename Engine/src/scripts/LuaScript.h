#pragma once

#include "core/Core.h"
#include "memory/Asset.h"

struct LuaScriptImportSettings : public AssetCreateDescriptor
{

};

struct LuaScriptAssetManager : public AssetManager
{
	bool copyFiles(const std::string& fileLocation, AssetInfo& aInfo) override;
	ResourceWrapper<Resource> load(AssetInfo& aInfo) override;
	void save(const AssetWrapper<Resource>& script, const AssetInfo& aInfo) override;
};

class EngineAPI LuaScript : public Resource
{
public:
	static AssetWrapper<LuaScript> import(const std::string& fileLocation, LuaScriptImportSettings settings = {});
	static ResourceWrapper<LuaScript> create();
	static void updateAsset(const AssetWrapper<LuaScript>& script, AssetUpdateDescriptor desc);

	std::string filepath;
};