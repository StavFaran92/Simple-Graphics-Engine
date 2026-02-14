#include "scripts/LuaScript.h"

#include "core/Factory.h"


ResourceWrapper<LuaScript> LuaScript::load(const std::string& fileLocation, LoadDescriptor desc)
{
	ResourceWrapper<LuaScript> luaScript = Factory<LuaScript>::create();
	luaScript->filepath = fileLocation;
	return luaScript;
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
