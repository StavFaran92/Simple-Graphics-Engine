#include "scripts/ScriptSystem.h"
#include "component/ScriptComponent.h"
#include "core/Logger.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp> // or #include "sol.hpp", whichever suits your needs

#include "scripts/LuaBindings.h"
#include "runtime/Entity.h"

struct LuaState
{
    Entity entity;
    sol::table script;
};

void LogDebug(const std::string& msg) {
    logDebug(msg);
}

class ScriptSystem::Impl
{
public:
    sol::state lua;
    std::vector<LuaState> scripts;

    void init()
    {
        lua.open_libraries(
            sol::lib::base, 
            sol::lib::math, 
            sol::lib::package);

        bindAll(lua);
    }
};

ScriptSystem::ScriptSystem()
    : impl_(std::make_unique<Impl>())
{
    Engine::get()->registerSubSystem<ScriptSystem>(this);
}

ScriptSystem::~ScriptSystem() = default;

void ScriptSystem::init()
{
    impl_->init();
}

void ScriptSystem::loadScript(ScriptComponent& scriptComponent)
{
    if (!scriptComponent.isValid())
        return;

    try {
        const std::filesystem::path projectDir = Engine::get()->getProjectDirectory();
        std::string filepath = (projectDir / scriptComponent.getScript()->filepath).string();
        impl_->lua.script_file(filepath);
        sol::table script = impl_->lua["Script"];
        if (script.valid())
        {
            impl_->scripts.push_back({ scriptComponent.entity, script });
        }
        else
        {
            logWarning("Warning: No 'Script' table found in {}", filepath);
        }

    
    }
    catch (const sol::error& e) 
    {
        logError("Error loading script: {}" ,e.what());
    }
}

void ScriptSystem::callCreate()
{
    for (auto& script : impl_->scripts)
    {
        sol::function fn = script.script["create"];
        if (fn.valid())
        {
            sol::protected_function_result result = fn(script.script, script.entity);
            if (!result.valid()) {
                sol::error err = result;
                logError("Lua Error: {}", err.what());
            }
        }
            
    }
}

void ScriptSystem::callUpdate(float dt)
{
    for (auto& script : impl_->scripts)
    {
        sol::function fn = script.script["update"];
        if (fn.valid())
        {
            fn(script.script, script.entity, dt);
        }
    }
}

void ScriptSystem::callDestroy()
{
    for (auto& script : impl_->scripts)
    {
        sol::function fn = script.script["destroy"];
        if (fn.valid())
            fn(script.script, script.entity);
    }

    impl_->scripts.clear();

}