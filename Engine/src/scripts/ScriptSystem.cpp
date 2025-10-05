#include "scripts/ScriptSystem.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp> // or #include "sol.hpp", whichever suits your needs

void ScriptSystem::init()
{
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.script("print('bark bark bark!')");
}