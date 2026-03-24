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
	std::ofstream os(dst.absolute());
	if (!os.is_open())
		return false;

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

	os << script;

	os.close();
	return true;
}

ResourceLoadDescriptor* LuaScriptTypeManager::makeResourceLoadDescriptor()
{
	return new LuaScriptLoadDescriptor();
}

ResourceWrapper<Resource> LuaScriptTypeManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	return ResourceWrapper<Resource>();
}

void LuaScriptTypeManager::parse(ResourceLoadDescriptor& desc)
{
}

void LuaScriptTypeManager::parse(ResourceBuildDescriptor& desc)
{
}


