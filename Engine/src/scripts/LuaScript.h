#pragma once

#include "core/Core.h"
#include "memory/Asset.h"

struct LuaScriptImportSettings : public AssetCreateDescriptor
{

};

// Asset IO Manager
struct LuaScriptAssetManager : public AssetManager
{
	bool copyFiles(const std::string& fileLocation, AssetRecord& aInfo) override;
	ResourceWrapper<Resource> load(AssetRecord& aInfo) override;
	void save(AssetHandle<Asset> asset, const AssetRecord& aInfo) override;
};

// Resource
class EngineAPI LuaScript : public Resource
{
public:
	static ResourceWrapper<LuaScript> create();
	static void updateAsset(const AssetHandle<LuaScript>& script, AssetUpdateDescriptor desc);

	std::string filepath;
};

// Asset
class EngineAPI LuaScriptAsset : public Asset
{
	using ResourceType = LuaScript;
public:
	static AssetHandle<LuaScriptAsset> import(const std::string& fileLocation, LuaScriptImportSettings aDesc = {});
	static void update(const AssetHandle<LuaScriptAsset>& script, AssetUpdateDescriptor desc);
};