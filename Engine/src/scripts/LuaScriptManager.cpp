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

bool LuaScriptTypeManager::importAsset(const std::string& src, const ScopedPath& dst)
{
	return std::filesystem::copy_file(src, dst.absolute(), std::filesystem::copy_options::overwrite_existing);
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


