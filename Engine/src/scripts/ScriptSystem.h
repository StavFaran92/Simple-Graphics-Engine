#pragma once
#include <memory>
#include <string>

#include "component/ScriptComponent.h"

class ScriptSystem
{
public:
    ScriptSystem();
    ~ScriptSystem();

    void init();

    void loadScript(ScriptComponent& scriptComponent);

    void callCreate(ScriptComponent& script);
    void callUpdate(ScriptComponent& script, float dt);
    void callDestroy(ScriptComponent& script);

private:
    class Impl;                 
    std::unique_ptr<Impl> impl_;
};