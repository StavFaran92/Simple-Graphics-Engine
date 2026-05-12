#pragma once
#include <memory>
#include <string>

#include "component/ScriptComponent.h"
#include "systems/SubSystem.h"

class ScriptSystem : public SubSystem
{
public:
    ScriptSystem();
    ~ScriptSystem();

    void init();

    void loadScript(ScriptComponent& scriptComponent);

    void callCreate();
    void callUpdate(float dt);
    void callOnEvent(Entity entity, SDL_Event event);
    void callDestroy();

private:
    class Impl;                 
    std::unique_ptr<Impl> impl_;
};