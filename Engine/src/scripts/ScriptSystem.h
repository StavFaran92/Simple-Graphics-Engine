#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "component/ScriptComponent.h"
#include "systems/SubSystem.h"
#include "core/Event.h"

using ScriptArg = std::variant<bool, int, float, std::string>;

class EngineAPI ScriptSystem : public SubSystem
{
public:
    enum class CollisionType
    {
        TRIGGER_ENTER,
        TRIGGER_EXIT,
        COLLISION_ENTER,
        COLLISION_EXIT
    };
public:
    ScriptSystem();
    ~ScriptSystem();

    void init();

    void reloadScript(Entity e, ScriptComponent& scriptComponent);

    void callCreate(Entity entity);
    void callCreateOnAll();
    void callUpdate(Entity entity, float dt);
    void callUpdateOnAll(float dt);
    void callOnEvent(Entity entity, const Event& event);
    void callOnCollide(CollisionType, Entity entity, Entity other);
    void callOnAnimTrigger(Entity entity, const std::string& name, int frameID);
    void callDestroy(Entity entity);
    void callDestroyOnAll();

    void resolveRefs(Entity entity);

    
    void invokeFunction(Entity entity, const std::string& funcName, const std::vector<ScriptArg>& args);
    //void setRef(Entity entity, const std::string& fieldName, Entity ref);
    //Entity getRef(Entity entity, const std::string& fieldName) const;

    // Returns discovered Ref() fields for the given script entity. nullptr if not found.
    const std::unordered_map<std::string, std::string>* getScriptRefs(Entity entity) const;

private:
    class Impl;                 
    std::unique_ptr<Impl> impl_;
};