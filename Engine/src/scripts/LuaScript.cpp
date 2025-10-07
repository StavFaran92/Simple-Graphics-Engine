#include "scripts/LuaScript.h"

#include "core/Factory.h"
#include <fstream>

namespace {
	struct LuaScriptManagerRegistration {
		LuaScriptManagerRegistration() {
			AssetFactory::registerManager(AssetType::LUA_SCRIPT, std::make_shared<LuaScriptAssetManager>());
		}
	} _luaScriptManagerRegistration;
}

bool LuaScriptAssetManager::copyFiles(const std::string& fileLocation, AssetInfo& aInfo)
{
	const std::filesystem::path projectDir = Engine::get()->getProjectDirectory();
	const std::string relativeFilepath = aInfo.name + aInfo.ext;
	const std::filesystem::path savedFilePath = projectDir / aInfo.assetDirectory / relativeFilepath;
	return std::filesystem::copy_file(fileLocation, savedFilePath);
}

ResourceWrapper<ResourceBase> LuaScriptAssetManager::load(AssetInfo& aInfo)
{
	//auto projectDir = Engine::get()->getProjectDirectory();
	//std::ifstream is(projectDir + "/" + aInfo.filePath);
	LuaScript* loadedScript = new LuaScript();
	loadedScript->filepath = aInfo.filePath;

	try
	{
		Engine::get()->getMemoryPool().add(aInfo.uuid, loadedScript);
		return ResourceWrapper<ResourceBase>(aInfo.uuid);

	}
	catch (const cereal::Exception& e)
	{
		logError("Deserialization Error occured: {}", e.what());
	}

	return ResourceWrapper<ResourceBase>::empty;
}

void LuaScriptAssetManager::save(const ResourceWrapper<ResourceBase>& script, const AssetInfo& aInfo)
{
	const std::filesystem::path projectDir = Engine::get()->getProjectDirectory();
	const std::string relativeFilepath = aInfo.name + aInfo.ext;
	const std::filesystem::path savedFilePath = projectDir / aInfo.assetDirectory / relativeFilepath;
	std::filesystem::copy_file(script.as<LuaScript>()->filepath, savedFilePath);
	script.as<LuaScript>()->filepath = savedFilePath.generic_string();
}

ResourceWrapper<LuaScript> LuaScript::import(const std::string& fileLocation, LuaScriptImportSettings desc)
{
	desc.aType = AssetType::LUA_SCRIPT;
	desc.origFilePath = fileLocation;
	return Engine::get()->getSubSystem<Assets>()->importAsset(fileLocation, desc).as<LuaScript>();
}

ResourceWrapper<LuaScript> LuaScript::create()
{
	ResourceWrapper<LuaScript> script = Factory<LuaScript>::create();

	static int counter = 0;
	std::string name = "temp_script_" + std::to_string(counter++) + ".lua";
	auto temp = std::filesystem::temp_directory_path() / "SGE" / name;
	std::filesystem::create_directories(temp.parent_path());

	std::string baseLuaScriptFilepath = SGE_ROOT_DIR + "Resources/Engine/Scripts/base_lua_file.lua";
	std::filesystem::copy_file(baseLuaScriptFilepath, temp);
	script->filepath = temp.generic_string();

	return script;
}

void LuaScript::updateAsset(const ResourceWrapper<LuaScript>& script, AssetUpdateDescriptor desc)
{
	Engine::get()->getSubSystem<Assets>()->updateAsset(script, desc);
}