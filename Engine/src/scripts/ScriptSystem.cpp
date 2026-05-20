#include "scripts/ScriptSystem.h"
#include "component/ScriptComponent.h"
#include "core/Logger.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp> // or #include "sol.hpp", whichever suits your needs

#include "scripts/LuaBindings.h"
#include "runtime/Entity.h"
#include "core/Engine.h"
#include "core/Event.h"
#include "core/MouseEvents.h"
#include "core/KeyboardEvents.h"

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
        std::string filepath = (projectDir / scriptComponent.getScript().resource()->filepath).string();
        auto result = impl_->lua.safe_script_file(filepath, sol::script_pass_on_error);
        if (!result.valid()) {
            sol::error err = result;
            logError(err.what());
            return;
        }
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
        sol::protected_function fn = script.script["create"];
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
        sol::protected_function fn = script.script["update"];
        if (fn.valid())
        {
            sol::protected_function_result result = fn(script.script, script.entity, dt);
            if (!result.valid()) {
                sol::error err = result;
                logError("Lua Error: {}", err.what());
            }
        }
    }
}

void ScriptSystem::callOnEvent(Entity entity, const Event& event)
{
    for (auto& state : impl_->scripts)
    {
        // TODO Should be optimized, no reason to iterate all the scripts for a single entity
        if (state.entity == entity)
        {
            sol::protected_function fn = state.script["onEvent"];
            if (fn.valid())
            {
                sol::protected_function_result result;

                //todo fix
                switch (event.type())
                {
                case EventType::MouseMoved:
                    result = fn(state.script, static_cast<const MouseMovedEvent*>(&event)); break;
                case EventType::KeyPressed:
                    result = fn(state.script, static_cast<const KeyPressedEvent*>(&event)); break;
                case EventType::KeyReleased:
                    result = fn(state.script, static_cast<const KeyReleasedEvent*>(&event)); break;
                case EventType::MouseButtonPressed:
                    result = fn(state.script, static_cast<const MouseButtonPressedEvent*>(&event)); break;
                case EventType::MouseButtonReleased:
                    result = fn(state.script, static_cast<const MouseButtonReleasedEvent*>(&event)); break;
                default:
                    result = fn(state.script, &event); break;
                }



                //sol::protected_function_result result = fn(state.script, &event);
                if (!result.valid()) {
                    sol::error err = result;
                    logError("Lua Error: {}", err.what());
                }
            }
        }

    }
}

void ScriptSystem::callOnCollide(CollisionType collisionType, Entity entity, Entity other)
{
    std::string funcName;
    if (collisionType == CollisionType::COLLISION_ENTER)
    {
        funcName = "onCollisionEnter";
    }
    else if (collisionType == CollisionType::COLLISION_EXIT)
    {
        funcName = "onCollisionExit";
    }
    if (collisionType == CollisionType::TRIGGER_ENTER)
    {
        funcName = "onTriggerEnter";
    }
    else if (collisionType == CollisionType::TRIGGER_EXIT)
    {
        funcName = "onTriggerExit";
    }

    assert(!funcName.empty());

    for (auto& state : impl_->scripts)
    {
        // TODO Should be optimized, no reason to iterate all the scripts for a single entity
        if (state.entity == entity)
        {
            sol::protected_function fn = state.script[funcName];
            if (fn.valid())
            {
                sol::protected_function_result result = fn(state.script, entity, other);

                if (!result.valid()) {
                    sol::error err = result;
                    logError("Lua Error: {}", err.what());
                }
            }
        }

        // TODO Should be optimized, no reason to iterate all the scripts for a single entity
        else if (state.entity == other)
        {
            sol::protected_function fn = state.script[funcName];
            if (fn.valid())
            {
                sol::protected_function_result result = fn(state.script, other, entity);

                if (!result.valid()) {
                    sol::error err = result;
                    logError("Lua Error: {}", err.what());
                }
            }
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