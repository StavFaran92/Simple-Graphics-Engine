#include "LuaScriptManager.h"

#include "scripts/LuaScript.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"

#include <filesystem>
#include <fstream>

Ref<Asset> LuaScriptTypeManager::createAsset(const AssetBuildDescriptor& assetDesc, const ResourceBuildDescriptor& resourceDesc)
{
	return createRef< LuaScriptAsset>();
}

Ref<Asset> LuaScriptTypeManager::deserializeAsset(const nlohmann::json& j)
{
	auto asset = createRef<LuaScriptAsset>();
	asset->deserialize(j);
	return asset;
}

bool LuaScriptTypeManager::importAsset(const std::string& src, ImportNode& result)
{
	std::filesystem::path path(src);
	result.name = path.filename().stem().string();
	result.assetDesc.aType = AssetType::LUA_SCRIPT;
	auto luaLoadDesc = result.emplaceLoadDesc<LuaScriptLoadDescriptor>();
	luaLoadDesc->sourcePath = src; 

	return true;
}

bool LuaScriptTypeManager::saveResource(const ResourceBuildDescriptor& desc, const ScopedPath& dst)
{
	auto luaScriptDesc = dynamic_cast<const LuaScriptCreateDescriptor*>(&desc);
	if (!luaScriptDesc)
	{
		logError("Invalid Descriptor specified.");
		return false;
	}

	std::ofstream os(dst.absolute());
	if (!os.is_open())
		return false;

	os << luaScriptDesc->script;

	os.close();

	return true;
}

ResourceLoadDescriptor* LuaScriptTypeManager::makeResourceLoadDescriptor()
{
	return new LuaScriptLoadDescriptor();
}

ResourceWrapper<Resource> LuaScriptTypeManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	auto luaDesc = dynamic_cast<const LuaScriptLoadDescriptor*>(&desc);
	if (!luaDesc)
	{
		logError("Invalid Descriptor specified.");
		return ResourceWrapper<Resource>::empty;
	}

	return LuaScript::load(desc.sourcePath, *luaDesc);
}

void LuaScriptTypeManager::parse(ResourceLoadDescriptor& desc)
{
}

void LuaScriptTypeManager::parse(ResourceBuildDescriptor& desc)
{
}


