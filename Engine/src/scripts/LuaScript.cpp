#include "scripts/LuaScript.h"

#include "core/Factory.h"
#include "serialize/CerealHelpers.h"

#include <cereal/archives/json.hpp>


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

void LuaScriptAsset::saveMetadata(cereal::JSONOutputArchive& archive) const
{
	SERIALIZED_MEMBER(filepath);
}

void LuaScriptAsset::loadMetadata(cereal::JSONInputArchive& archive)
{
	SERIALIZED_MEMBER_OPTIONAL(filepath);
}
