#pragma once

#include <string>
#include "core/Core.h"

class EngineAPI State {
public:
    std::string name;

    State(const std::string& name) : name(name) {}
    virtual ~State() = default;

    virtual void onEnter() {}
    virtual void onUpdate(float dt) {}
    virtual void onExit() {}
};