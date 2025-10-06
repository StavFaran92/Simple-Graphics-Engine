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

    void callCreate();
    void callUpdate(float dt);
    void callDestroy();

private:
    class Impl;                 
    std::unique_ptr<Impl> impl_;
};