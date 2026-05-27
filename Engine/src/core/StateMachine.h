#pragma once

#include "State.h"
#include "core/Core.h"
#include <unordered_map>
#include <memory>
#include <string>

class EngineAPI StateMachine {
    std::unordered_map<std::string, std::shared_ptr<State>> states;
    std::shared_ptr<State> current = nullptr;

public:
    void addState(std::shared_ptr<State> state) {
        states[state->name] = state;
    }

    void transitionTo(const std::string& name) {
        auto it = states.find(name);
        if (it == states.end()) 
            return;

        if (current) 
            current->onExit();
        current = it->second;
        current->onEnter();
    }

    void update(float dt) {
        if (current) 
            current->onUpdate(dt);
    }

    std::string currentState() const {
        return current ? current->name : "";
    }
};