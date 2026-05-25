#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "component/ScriptComponent.h"
#include "systems/SubSystem.h"
#include "core/Event.h"

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

    void loadScript(ScriptComponent& scriptComponent);

    void callCreate();
    void callUpdate(float dt);
    void callOnEvent(Entity entity, const Event& event);
    void callOnCollide(CollisionType, Entity entity, Entity other);
    void callOnAnimTrigger(Entity entity, const std::string& name, int frameID);
    void callDestroy();

    // Returns discovered Ref() fields for the given script entity. nullptr if not found.
    const std::unordered_map<std::string, std::string>* getScriptRefs(Entity entity) const;

private:
    class Impl;                 
    std::unique_ptr<Impl> impl_;
};