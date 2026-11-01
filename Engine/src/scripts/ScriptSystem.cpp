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
            sol::lib::package,
            sol::lib::table,
            sol::lib::string,
            sol::lib::utf8,
            sol::lib::os);

        // Scripts get the clock, not the shell or the file system
        sol::table os = lua["os"];
        for (const char* name : { "execute", "exit", "remove", "rename", "getenv", "tmpname", "setlocale" })
            os[name] = sol::lua_nil;

        bindAll(lua);
        lua.set_function("invoke", &Impl::invoke, this);
    }

    void bindDynamics()
    {
        bindDynamic(lua);
    }

    void invoke(Entity target, const std::string& funcName, sol::variadic_args args);
};

ScriptSystem::ScriptSystem()
    : impl_(std::make_unique<Impl>())
{
}

ScriptSystem::~ScriptSystem() = default;

void ScriptSystem::init()
{
    impl_->init();
}

void ScriptSystem::bindDynamics()
{
    impl_->bindDynamics();
}

// A Lua error can be any value; converting a non-string one to sol::error throws
static void logLuaError(const sol::protected_function_result& result)
{
    sol::object err = result.get<sol::object>();
    if (err.is<std::string>())
    {
        logError("Lua Error: {}", err.as<std::string>());
        return;
    }

    sol::protected_function tostring = sol::state_view(err.lua_state())["tostring"];
    sol::protected_function_result str = tostring(err);
    if (str.valid() && str.get_type() == sol::type::string)
        logError("Lua Error: {}", str.get<std::string>());
    else
        logError("Lua Error: (non-string error of type {})", sol::type_name(err.lua_state(), err.get_type()));
}

// Runs the script file and extracts its Script table and Ref() fields into outState, without
// registering anything. Returns false if the script is invalid or fails to load.
static bool loadScriptState(sol::state& lua, ScriptComponent& scriptComponent, LuaState& outState)
{
    if (!scriptComponent.isValid())
        return false;

    try {
        // The resource filepath is already resolved by the loader (it includes the project dir),
        // only fall back to joining with the project dir for a bare project-relative path.
        std::filesystem::path scriptPath = scriptComponent.getScript().resource()->filepath;
        if (scriptPath.is_relative() && !std::filesystem::exists(scriptPath))
            scriptPath = std::filesystem::path(Engine::get()->getProjectDirectory()) / scriptPath;
        std::string filepath = scriptPath.string();
        auto result = lua.safe_script_file(filepath, sol::script_pass_on_error);
        if (!result.valid()) {
            logLuaError(result);
            return false;
        }
        sol::table script = lua["Script"];
        if (!script.valid())
        {
            logWarning("Warning: No 'Script' table found in {}", filepath);
            return false;
        }

        outState = LuaState{ script };

        for (auto& [key, value] : script) {
            if (!key.is<std::string>() || !value.is<sol::table>())
                continue;
            sol::table t = value.as<sol::table>();
            auto isRef = t.get<sol::optional<bool>>("__isRef");
            if (isRef && *isRef) {
                std::string fieldName = key.as<std::string>();
                std::string refType = t.get_or<std::string>("refType", "Entity");
                outState.refs[fieldName] = refType;
                script[fieldName] = sol::nil;
            }
        }

        return true;
    }
    catch (const sol::error& e)
    {
        logError("Error loading script: {}" ,e.what());
    }

    return false;
}

void ScriptSystem::reloadScript(Entity e, ScriptComponent& scriptComponent)
{
    LuaState state;
    if (loadScriptState(impl_->lua, scriptComponent, state))
    {
        impl_->scripts[e] = std::move(state);
    }
}

bool ScriptSystem::getScriptRefSlots(ScriptComponent& scriptComponent, std::unordered_map<std::string, std::string>& outRefSlots)
{
    LuaState state;
    if (!loadScriptState(impl_->lua, scriptComponent, state))
        return false;

    outRefSlots = std::move(state.refs);
    return true;
}

void ScriptSystem::callCreate(Entity entity)
{
    auto it = impl_->scripts.find(entity);
    if (it == impl_->scripts.end())
        return;

    sol::protected_function fn = it->second.script["create"];
    if (fn.valid())
    {
        sol::protected_function_result result = fn(it->second.script, Entity(entity));
        if (!result.valid()) {
            logLuaError(result);
        }
    }
}

void ScriptSystem::callCreateOnAll()
{
    // Snapshot entities first: calling into Lua's create() may spawn a prefab,
    // which inserts/erases entries in impl_->scripts (see ScriptComponent::postLoad).
    // Iterating the map directly while that happens is UB (insert can rehash and
    // invalidate the in-flight iterator).
    std::vector<Entity> entities;
    entities.reserve(impl_->scripts.size());
    for (auto& [e, state] : impl_->scripts)
        entities.push_back(e);

    for (Entity e : entities)
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
        sol::protected_function_result result = fn(it->second.script, Entity(entity), dt);
        if (!result.valid()) {
            logLuaError(result);
        }
    }
}

void ScriptSystem::callUpdateOnAll(float dt)
{
    // See callCreateOnAll: snapshot entities first since update() can spawn
    // prefabs and mutate impl_->scripts mid-iteration.
    std::vector<Entity> entities;
    entities.reserve(impl_->scripts.size());
    for (auto& [e, state] : impl_->scripts)
        entities.push_back(e);

    for (Entity e : entities)
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
            logLuaError(result);
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
            sol::protected_function_result result = fn(it->second.script, Entity(self), Entity(opponent));
            if (!result.valid()) {
                logLuaError(result);
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
            logLuaError(result);
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

    sol::protected_function fn = it->second.script["destroy"];
    if (fn.valid())
    {
        sol::protected_function_result result = fn(it->second.script, Entity(entity));
        if (!result.valid())
            logLuaError(result);
    }

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

// Lua's invoke(target, funcName, ...): calls funcName on target's script with the caller's arguments as they are
void ScriptSystem::Impl::invoke(Entity target, const std::string& funcName, sol::variadic_args args)
{
    auto it = scripts.find(target);
    if (it == scripts.end())
        return;

    sol::protected_function fn = it->second.script[funcName];
    if (!fn.valid())
        return;

    sol::protected_function_result result = fn(it->second.script, args);
    if (!result.valid())
        logLuaError(result);
}
