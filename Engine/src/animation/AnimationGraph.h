#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "core/Core.h"
#include "animation/Animator.h"
#include "nlohmann/json.hpp"

class Animator;

// ---- Enums ----

enum class ParameterType { Float, Bool, Int };

enum class ConditionType
{
    OnValueEqual,
    OnValueGreaterThan,
    OnValueLessThan,
    OnAnimationEnd
};

// ---- Data structs ----

struct Parameter
{
    std::string name;
    ParameterType type = ParameterType::Float;
    float defaultValue = 0.f;
};

struct Condition
{
    ConditionType type = ConditionType::OnAnimationEnd;
    std::string parameter;
    float value = 0.f;
};

struct StateNode
{
    std::string id;
    AnimationEntry animationEntry;
    bool loop = true;
};

struct Transition
{
    std::string from;  // source state id, or "__any_state__"
    std::string to;
    float blendDuration = 0.2f;
    std::vector<Condition> conditions;  // AND logic, evaluated in order
};

// ---- AnimationGraph ----

class EngineAPI AnimationGraph
{
public:
    static constexpr const char* ANY_STATE_ID = "__any_state__";

    AnimationGraph() = default;
    void setAnimatorOwner(Animator* animator);

    // Build API (code-side authoring)
    void addState(StateNode state);
    void addTransition(Transition transition);
    void addParameter(Parameter param);
    void setEntryState(const std::string& stateId);

    // Runtime update — call from Animator::update
    void update(float dt);

    // Called by Animator when a non-looping animation reaches its end
    void onAnimationEnd();

    // Parameter setters for game code
    void setFloat(const std::string& name, float value);
    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);

    // JSON serialization
    void loadFromJson(const nlohmann::json& j);
    nlohmann::json saveToJson() const;

    const StateNode* getCurrentState() const;

private:
    bool evaluateCondition(const Condition& cond) const;
    bool evaluateTransitions(const std::vector<Transition*>& transitions);
    void transitionTo(const std::string& stateId, float blendDuration);
    const StateNode* findState(const std::string& id) const;

private:
    Animator* m_owner = nullptr;

    std::vector<StateNode> m_states;
    std::vector<Transition> m_transitions;
    std::vector<Parameter> m_parameters;
    std::unordered_map<std::string, float> m_paramValues;

    std::string m_currentStateId;
    std::string m_entryStateId;
    bool m_animationEndedThisFrame = false;
};
