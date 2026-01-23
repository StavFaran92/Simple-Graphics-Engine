#pragma once

#include "core/Core.h"
#include "memory/Asset.h"

struct LuaScriptImportSettings : public AssetCreateDescriptor
{

};

struct LuaScriptAssetManager : public AssetManager
{
	bool copyFiles(const std::string& fileLocation, AssetRecord& aInfo) override;
	ResourceWrapper<Resource> load(AssetRecord& aInfo) override;
	void save(const AssetHandle<Resource>& script, const AssetRecord& aInfo) override;
};

class EngineAPI LuaScript : public Resource
{
public:
	static AssetHandle<LuaScript> import(const std::string& fileLocation, LuaScriptImportSettings settings = {});
	static ResourceWrapper<LuaScript> create();
	static void updateAsset(const AssetHandle<LuaScript>& script, AssetUpdateDescriptor desc);

	std::string filepath;
};