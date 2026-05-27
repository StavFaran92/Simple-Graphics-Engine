#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "core/Core.h"
#include "nlohmann/json.hpp"
#include "serialize/CerealHelpers.h"
#include "animation/AnimationEntry.h"


class Animator;

// ---- Enums ----

enum class ParameterType { Float, Bool, Int, Trigger };

enum class ConditionType
{
    OnValueEqual,
    OnValueGreaterThan,
    OnValueLessThan,
    OnAnimationEnd,
    OnTrigger
};

// ---- Data structs ----

struct Parameter
{
    std::string name;
    ParameterType type = ParameterType::Float;
    float defaultValue = 0.f;

    template <class Archive>
    void serialize(Archive& archive) {
        SERIALIZED_MEMBER_OPTIONAL(name);
        SERIALIZED_MEMBER_OPTIONAL(type);
        SERIALIZED_MEMBER_OPTIONAL(defaultValue);
    }
};

struct Condition
{
    ConditionType type = ConditionType::OnAnimationEnd;
    std::string parameter;
    float value = 0.f;

    template <class Archive>
    void serialize(Archive& archive) {
        SERIALIZED_MEMBER_OPTIONAL(type);
        SERIALIZED_MEMBER_OPTIONAL(parameter);
        SERIALIZED_MEMBER_OPTIONAL(value);
    }
};

struct StateNode
{
    std::string id;
    AnimationEntry animationEntry;
    bool loop = true;
    glm::vec2 pos;

    template <class Archive>
    void serialize(Archive& archive) {
        SERIALIZED_MEMBER_OPTIONAL(id);
        SERIALIZED_MEMBER_OPTIONAL(animationEntry);
        SERIALIZED_MEMBER_OPTIONAL(loop);
        SERIALIZED_MEMBER_OPTIONAL(pos);
    }
};

struct Transition
{
    std::string from;  // source state id, or "__any_state__"
    std::string to;
    float blendDuration = 0.2f;
    std::vector<Condition> conditions;  // AND logic, evaluated in order

    template <class Archive>
    void serialize(Archive& archive) {
        SERIALIZED_MEMBER_OPTIONAL(from);
        SERIALIZED_MEMBER_OPTIONAL(to);
        SERIALIZED_MEMBER_OPTIONAL(blendDuration);
        SERIALIZED_MEMBER_OPTIONAL(conditions);
    }
};

// ---- AnimationGraph ----

class EngineAPI AnimationGraph
{
public:
    static constexpr const char* ANY_STATE_ID = "__any_state__";

    AnimationGraph() = default;
    void setAnimatorOwner(Animator* animator);
    void init();

    // Build API (code-side authoring)
    void addState(StateNode state);
    void addTransition(Transition transition);
    void addParameter(Parameter param);
    void setEntryState(const std::string& stateId);
    void removeState(size_t index);

    // Runtime update — call from Animator::update
    void update(float dt);

    // Called by Animator when a non-looping animation reaches its end
    void onAnimationEnd();

    // Parameter setters for game code
    void setFloat(const std::string& name, float value);
    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);
    void trigger(const std::string& name);

    // JSON serialization
    void loadFromJson(const nlohmann::json& j);
    nlohmann::json saveToJson() const;

    const StateNode* getCurrentState() const;

    // Editor read access
    const std::vector<StateNode>&  getStates()      const { return m_states; }
    const std::vector<Transition>& getTransitions()  const { return m_transitions; }
    const std::vector<Parameter>&  getParameters()   const { return m_parameters; }
    std::string getEntryState()   const { return m_entryStateId; }

    // Editor write access
    StateNode*  getState(size_t index)      { return index < m_states.size()      ? &m_states[index]      : nullptr; }
    Transition* getTransition(size_t index) { return index < m_transitions.size() ? &m_transitions[index] : nullptr; }
    Parameter*  getParameter(size_t index)  { return index < m_parameters.size()  ? &m_parameters[index]  : nullptr; }

    glm::vec2 getNodeCenter(size_t index);
    void setNodeCenter(size_t index, glm::vec2 pos);

    void removeParameter(size_t index)
    {
        if (index >= m_parameters.size()) return;
        m_paramValues.erase(m_parameters[index].name);
        m_parameters.erase(m_parameters.begin() + index);
    }

    void removeTransition(size_t index)
    {
        if (index < m_transitions.size())
            m_transitions.erase(m_transitions.begin() + index);
    }

    

    template <class Archive>
    void serialize(Archive& archive) {
        SERIALIZED_MEMBER_OPTIONAL(m_states);
        SERIALIZED_MEMBER_OPTIONAL(m_transitions);
        SERIALIZED_MEMBER_OPTIONAL(m_parameters);
        SERIALIZED_MEMBER_OPTIONAL(m_entryStateId);
    }

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
    std::unordered_set<std::string> m_pendingTriggers;

    std::string m_currentStateId;
    std::string m_entryStateId;
    bool m_animationEndedThisFrame = false;
};
