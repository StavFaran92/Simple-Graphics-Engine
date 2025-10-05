#include "scripts/ScriptSystem.h"
#include "component/ScriptComponent.h"
#include "core/Logger.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp> // or #include "sol.hpp", whichever suits your needs

//struct LuaState
//{
//    sol::table script
//};

class ScriptSystem::Impl
{
public:
    sol::state lua;
    std::unordered_map<int, sol::table> scripts;

    void init()
    {
        lua.open_libraries(sol::lib::base);
    }

    int counter = 0;

    int count()
    {
        return counter++;
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

void ScriptSystem::loadScript(const std::string& path, ScriptComponent& scriptComponent)
{
    try {
        impl_->lua.script_file(path);
        sol::table script = impl_->lua["Script"];
        if (script.valid())
        {
            if (scriptComponent.handlerID == -1)
            {
                scriptComponent.handlerID = impl_->count();
            }
            impl_->scripts[scriptComponent.handlerID] = script;
            scriptComponent.filepath = path;
        }
        else
        {
            logWarning("Warning: No 'Script' table found in {}", scriptComponent.filepath);
        }

    
    }
    catch (const sol::error& e) {
        logError("Error loading script: {}" ,e.what());
    }
}

void ScriptSystem::callCreate(ScriptComponent& scriptComponent)
{
    if (scriptComponent.isValid())
    {
        sol::table& script = impl_->scripts[scriptComponent.handlerID];
        sol::function fn = script["create"];
        if (fn.valid())
            fn(script);
    }
}

void ScriptSystem::callUpdate(ScriptComponent& scriptComponent, float dt)
{
    if (scriptComponent.isValid())
    {
        sol::table& script = impl_->scripts[scriptComponent.handlerID];
        sol::function fn = script["update"];
        if (fn.valid())
            fn(script, dt);
    }
}

void ScriptSystem::callDestroy(ScriptComponent& scriptComponent)
{
    if (scriptComponent.isValid())
    {
        sol::table& script = impl_->scripts[scriptComponent.handlerID];
        sol::function fn = script["destroy"];
        if (fn.valid())
            fn(script);
    }

}