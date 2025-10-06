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
	throw std::runtime_error("Not yet implemented!");
}

ResourceWrapper<LuaScript> LuaScript::import(const std::string& fileLocation, LuaScriptImportSettings desc)
{
	desc.aType = AssetType::LUA_SCRIPT;
	desc.origFilePath = fileLocation;
	return Engine::get()->getSubSystem<Assets>()->importAsset(fileLocation, desc).as<LuaScript>();
}

ResourceWrapper<LuaScript> LuaScript::create()
{
	return Factory<LuaScript>::create();
}

void LuaScript::updateAsset(const ResourceWrapper<LuaScript>& script, AssetUpdateDescriptor desc)
{
	Engine::get()->getSubSystem<Assets>()->updateAsset(script, desc);
}