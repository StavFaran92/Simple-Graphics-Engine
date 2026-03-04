#include "LuaScriptManager.h"

#include "scripts/LuaScript.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"

#include <filesystem>

Asset* LuaScriptTypeManager::createAsset(AssetCreateDescriptor& desc)
{
	return new LuaScriptAsset(desc);
}

bool LuaScriptTypeManager::importAsset(const std::string& src, ImportNode& result)
{
	std::filesystem::path path(src);
	result.name = path.filename().stem().string();
	result.createDescriptor.aType = AssetType::LUA_SCRIPT;
	result.createDescriptor.sourcePath = src;
	result.createDescriptor.makeResourceLoadDescriptor<LuaScriptLoadDescriptor>()->sourcePath = src; //todo fix

	return true;
}

bool LuaScriptTypeManager::saveResource(const ResourceCreateDescriptor& desc, const ScopedPath& dst)
{
	return false;
}

ResourceLoadDescriptor* LuaScriptTypeManager::makeResourceLoadDescriptor()
{
	return nullptr;
}

ResourceWrapper<Resource> LuaScriptTypeManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	return ResourceWrapper<Resource>();
}

void LuaScriptTypeManager::parse(ResourceLoadDescriptor& desc)
{
}

void LuaScriptTypeManager::parse(ResourceCreateDescriptor& desc)
{
}


