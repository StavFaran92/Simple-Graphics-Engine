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
    sol::table script;
    std::unordered_map<std::string, std::string> refs;
};

class ScriptSystem::Impl
{
public:
    sol::state lua;
    std::unordered_map<Entity, LuaState> scripts;

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

void ScriptSystem::reloadScript(Entity e, ScriptComponent& scriptComponent)
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
            LuaState state{ script };

            for (auto& [key, value] : script) {
                if (!key.is<std::string>() || !value.is<sol::table>()) 
                    continue;
                sol::table t = value.as<sol::table>();
                auto isRef = t.get<sol::optional<bool>>("__isRef");
                if (isRef && *isRef) {
                    std::string fieldName = key.as<std::string>();
                    std::string refType = t.get_or<std::string>("refType", "Entity");
                    state.refs[fieldName] = refType;
                    script[fieldName] = sol::nil;
                }
            }

            impl_->scripts[scriptComponent.entity] = std::move(state);
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

void ScriptSystem::callCreate(Entity entity)
{
    auto it = impl_->scripts.find(entity);
    if (it == impl_->scripts.end())
        return;

    sol::protected_function fn = it->second.script["create"];
    if (fn.valid())
    {
        sol::protected_function_result result = fn(it->second.script, entity);
        if (!result.valid()) {
            sol::error err = result;
            logError("Lua Error: {}", err.what());
        }
    }
}

void ScriptSystem::callCreateOnAll()
{
    for (auto& [e, state] : impl_->scripts)
        callCreate(e);
}

void ScriptSystem::callUpdate(Entity entity, float dt)
{
    auto it = impl_->scripts.find(entity);
    if (it == impl_->scripts.end())
        return;

    sol::protected_function fn = it->second.script["update"];
    if (fn.valid())
    {
        sol::protected_function_result result = fn(it->second.script, entity, dt);
        if (!result.valid()) {
            sol::error err = result;
            logError("Lua Error: {}", err.what());
        }
    }
}

void ScriptSystem::callUpdateOnAll(float dt)
{
    for (auto& [e, state] : impl_->scripts)
        callUpdate(e, dt);
}

void ScriptSystem::callOnEvent(Entity entity, const Event& event)
{
    auto it = impl_->scripts.find(entity);
    if (it == impl_->scripts.end())
        return;

    LuaState& state = it->second;
    sol::protected_function fn = state.script["onEvent"];
    if (fn.valid())
    {
        sol::protected_function_result result;
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

        if (!result.valid()) {
            sol::error err = result;
            logError("Lua Error: {}", err.what());
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

    auto callFn = [&](Entity self, Entity opponent) {
        auto it = impl_->scripts.find(self);
        if (it == impl_->scripts.end())
            return;
        sol::protected_function fn = it->second.script[funcName];
        if (fn.valid())
        {
            sol::protected_function_result result = fn(it->second.script, self, opponent);
            if (!result.valid()) {
                sol::error err = result;
                logError("Lua Error: {}", err.what());
            }
        }
    };

    callFn(entity, other);
    callFn(other, entity);
}

void ScriptSystem::callOnAnimTrigger(Entity entity, const std::string& name, int frameID)
{
    auto it = impl_->scripts.find(entity);
    if (it == impl_->scripts.end())
        return;

    sol::protected_function fn = it->second.script["onAnimationTrigger"];
    if (fn.valid())
    {
        sol::protected_function_result result = fn(it->second.script, name, frameID);
        if (!result.valid()) {
            sol::error err = result;
            logError("Lua Error: {}", err.what());
        }
    }
}

const std::unordered_map<std::string, std::string>* ScriptSystem::getScriptRefs(Entity entity) const
{
    auto it = impl_->scripts.find(entity);
    if (it != impl_->scripts.end())
        return &it->second.refs;
    return nullptr;
}

void ScriptSystem::callDestroy(Entity entity)
{
    auto it = impl_->scripts.find(entity);
    if (it == impl_->scripts.end())
        return;

    sol::function fn = it->second.script["destroy"];
    if (fn.valid())
        fn(it->second.script, entity);

    impl_->scripts.erase(it);
}

void ScriptSystem::callDestroyOnAll()
{
    std::vector<Entity> entities;
    entities.reserve(impl_->scripts.size());
    for (auto& [e, state] : impl_->scripts)
        entities.push_back(e);

    for (Entity e : entities)
        callDestroy(e);
}

void ScriptSystem::resolveRefs(Entity entity)
{
    // If not valid script ignore
    auto it = impl_->scripts.find(entity);
    if (it == impl_->scripts.end())
        return;

    auto& scriptComponent = entity.getComponent<ScriptComponent>();
    auto refSlots = scriptComponent.getAllRefSlots();
    for (const auto& fieldName : refSlots)
    {
        it->second.script[fieldName] = scriptComponent.getRef(fieldName);
    }
}

static sol::object toSol(sol::state& lua, const ScriptArg& arg)
{
    return std::visit([&](auto&& val) -> sol::object {
        return sol::make_object(lua, val);
        }, arg);
}

void ScriptSystem::invokeFunction(Entity entity, const std::string& funcName, const std::vector<ScriptArg>& args)
{
    // If not valid script ignore
    auto it = impl_->scripts.find(entity);
    if (it == impl_->scripts.end())
        return;

    auto target = it->second.script;

    sol::protected_function fn = it->second.script[funcName];
    if (fn.valid())
    {
        // build sol call args
        sol::protected_function_result result;
        switch (args.size())
        {
        case 0: result = fn(target); break;
        case 1: result = fn(target, toSol(impl_->lua, args[0])); break;
        case 2: result = fn(target, toSol(impl_->lua, args[0]), toSol(impl_->lua, args[1])); break;
        case 3: result = fn(target, toSol(impl_->lua, args[0]), toSol(impl_->lua, args[1]), toSol(impl_->lua, args[2])); break;
        default:
        {
            // pack into table for larger arg counts
            sol::table argsTable = impl_->lua.create_table();
            for (size_t i = 0; i < args.size(); i++)
                argsTable[i + 1] = toSol(impl_->lua, args[i]);
            result = fn(target, argsTable);
            break;
        }
        }

        if (!result.valid()) 
        {
            sol::error err = result;
            logError("Lua Error: {}", err.what());
        }
    }
}
