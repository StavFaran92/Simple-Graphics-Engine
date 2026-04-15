#include "scripts/LuaScript.h"

#include "core/Factory.h"


LuaScriptResourceRef LuaScript::load(const std::string& fileLocation, LuaScriptLoadDescriptor desc)
{
	LuaScriptResourceRef luaScript = Factory<LuaScript>::create();
	luaScript->filepath = fileLocation;
	return luaScript;
}

LuaScriptResourceRef LuaScript::create()
{
	LuaScriptResourceRef script = Factory<LuaScript>::create();

	static int counter = 0;
	std::string name = "temp_script_" + std::to_string(counter++) + ".lua";
	auto temp = std::filesystem::temp_directory_path() / "SGE" / name;
	std::filesystem::create_directories(temp.parent_path());

	std::string baseLuaScriptFilepath = SGE_ROOT_DIR "Resources/Engine/Scripts/base_lua_file.lua";
	std::filesystem::copy_file(baseLuaScriptFilepath, temp, std::filesystem::copy_options::overwrite_existing);
	script->filepath = temp.generic_string();

	return script;
}

// ============================================================
//  LuaScriptAsset (Asset wrapper)
// ============================================================

void LuaScriptAsset::serialize(nlohmann::json& j) const
{
	// Persist the script filepath so the editor/runtime can restore
	// which script file this asset points to.
	j = nlohmann::json::object();
	j["filepath"] = filepath;
}

void LuaScriptAsset::deserialize(const nlohmann::json& j)
{
	if (j.contains("filepath"))
	{
		filepath = j.at("filepath").get<std::string>();
	}
}
