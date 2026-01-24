#include "scripts/LuaScript.h"

#include "core/Factory.h"

namespace {
	struct LuaScriptManagerRegistration {
		LuaScriptManagerRegistration() {
			AssetFactory::registerManager(AssetType::LUA_SCRIPT, std::make_shared<LuaScriptAssetManager>());
		}
	} _luaScriptManagerRegistration;
}

bool LuaScriptAssetManager::copyFiles(const std::string& fileLocation, AssetRecord& aInfo)
{
	const std::filesystem::path projectDir = Engine::get()->getProjectDirectory();
	const std::filesystem::path savedFilePath = projectDir / aInfo.relativefilePath;
	return std::filesystem::copy_file(fileLocation, savedFilePath);
}

ResourceWrapper<Resource> LuaScriptAssetManager::load(AssetRecord& aInfo)
{
	ResourceWrapper<LuaScript> luaScript = Factory<LuaScript>::create();
	luaScript->filepath = aInfo.relativefilePath;

	try
	{
		return luaScript;

	}
	catch (const cereal::Exception& e)
	{
		logError("Deserialization Error occured: {}", e.what());
	}

	return ResourceWrapper<Resource>::empty;
}

void LuaScriptAssetManager::save(AssetHandle<Asset> asset, const AssetRecord& aInfo)
{
	const std::filesystem::path projectDir = Engine::get()->getProjectDirectory();
	const std::filesystem::path savedFilePath = projectDir / aInfo.relativefilePath;
	std::filesystem::copy_file(asset.as<LuaScriptAsset>().resource()->filepath, savedFilePath);
	asset.as<LuaScriptAsset>().resource()->filepath = savedFilePath.generic_string();
}

ResourceWrapper<LuaScript> LuaScript::create()
{
	ResourceWrapper<LuaScript> script = Factory<LuaScript>::create();

	static int counter = 0;
	std::string name = "temp_script_" + std::to_string(counter++) + ".lua";
	auto temp = std::filesystem::temp_directory_path() / "SGE" / name;
	std::filesystem::create_directories(temp.parent_path());

	std::string baseLuaScriptFilepath = SGE_ROOT_DIR "Resources/Engine/Scripts/base_lua_file.lua";
	std::filesystem::copy_file(baseLuaScriptFilepath, temp, std::filesystem::copy_options::overwrite_existing);
	script->filepath = temp.generic_string();

	return script;
}

AssetHandle<LuaScriptAsset> LuaScriptAsset::import(const std::string& fileLocation, LuaScriptImportSettings desc)
{
	desc.aType = AssetType::LUA_SCRIPT;
	desc.origFilePath = fileLocation;
	return Engine::get()->getSubSystem<Assets>()->importAsset(fileLocation, desc).as<LuaScriptAsset>();
}

void LuaScriptAsset::update(const AssetHandle<LuaScriptAsset>& script, AssetUpdateDescriptor desc)
{
	Engine::get()->getSubSystem<Assets>()->updateAsset(script, desc);
}
