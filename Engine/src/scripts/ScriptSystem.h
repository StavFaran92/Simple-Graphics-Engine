#pragma once
#include <memory>
#include <string>

#include "component/ScriptComponent.h"
#include "systems/SubSystem.h"
#include "core/Event.h"

class ScriptSystem : public SubSystem
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
    void callDestroy();

private:
    class Impl;                 
    std::unique_ptr<Impl> impl_;
};