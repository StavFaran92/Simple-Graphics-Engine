#include "animation/AnimationGraph.h"
#include "animation/Animator.h"
#include "core/Logger.h"

// ---- String conversion helpers ----

static ParameterType paramTypeFromString(const std::string& s)
{
    if (s == "bool")    return ParameterType::Bool;
    if (s == "int")     return ParameterType::Int;
    if (s == "trigger") return ParameterType::Trigger;
    return ParameterType::Float;
}

static std::string paramTypeToString(ParameterType t)
{
    switch (t)
    {
        case ParameterType::Bool:    return "bool";
        case ParameterType::Int:     return "int";
        case ParameterType::Trigger: return "trigger";
        default:                     return "float";
    }
}

static ConditionType condTypeFromString(const std::string& s)
{
    if (s == "onValueEqual")       return ConditionType::OnValueEqual;
    if (s == "onValueGreaterThan") return ConditionType::OnValueGreaterThan;
    if (s == "onValueLessThan")    return ConditionType::OnValueLessThan;
    if (s == "onTrigger")          return ConditionType::OnTrigger;
    return ConditionType::OnAnimationEnd;
}

static std::string condTypeToString(ConditionType t)
{
    switch (t)
    {
        case ConditionType::OnValueEqual:       return "onValueEqual";
        case ConditionType::OnValueGreaterThan: return "onValueGreaterThan";
        case ConditionType::OnValueLessThan:    return "onValueLessThan";
        case ConditionType::OnTrigger:          return "onTrigger";
        default:                                return "onAnimationEnd";
    }
}

// ---- AnimationGraph ----

void AnimationGraph::setAnimatorOwner(Animator* owner)
{
    m_owner = owner;
}

void AnimationGraph::init()
{
    transitionTo(m_entryStateId, 0.0f);
}

void AnimationGraph::addState(StateNode state)
{
    m_states.push_back(std::move(state));

    // only node avaiable -> make it entry node
    if (m_states.size() == 1)
    {
        setEntryState(state.id);
    }
}

void AnimationGraph::addTransition(Transition transition)
{
    m_transitions.push_back(std::move(transition));
}

void AnimationGraph::addParameter(Parameter param)
{
    m_paramValues[param.name] = param.defaultValue;
    m_parameters.push_back(std::move(param));
}

void AnimationGraph::setEntryState(const std::string& stateId)
{
    if (!findState(stateId))
    {
        logWarning("Invalid state ID: {}", stateId);
        return;
    }
    m_entryStateId = stateId;
    if (m_currentStateId.empty())
    {
        transitionTo(stateId, 0.f);
    }
}

void AnimationGraph::update(float dt)
{
    if (m_currentStateId.empty())
        return;

    // Collect transitions to evaluate: current-state first, then any-state
    std::vector<Transition*> candidates;
    for (auto& t : m_transitions)
    {
        if (t.from == m_currentStateId)
            candidates.push_back(&t);
    }
    for (auto& t : m_transitions)
    {
        if (t.from == ANY_STATE_ID && t.to != m_currentStateId)
            candidates.push_back(&t);
    }

    evaluateTransitions(candidates);
    m_animationEndedThisFrame = false;
    m_pendingTriggers.clear();
}

void AnimationGraph::onAnimationEnd()
{
    m_animationEndedThisFrame = true;
}

void AnimationGraph::setFloat(const std::string& name, float value)
{
    m_paramValues[name] = value;
}

void AnimationGraph::setBool(const std::string& name, bool value)
{
    m_paramValues[name] = value ? 1.f : 0.f;
}

void AnimationGraph::setInt(const std::string& name, int value)
{
    m_paramValues[name] = static_cast<float>(value);
}

void AnimationGraph::trigger(const std::string& name)
{
    m_pendingTriggers.insert(name);
}

const StateNode* AnimationGraph::getCurrentState() const
{
    return findState(m_currentStateId);
}

// ---- Private ----

bool AnimationGraph::evaluateCondition(const Condition& cond) const
{
    if (cond.type == ConditionType::OnAnimationEnd)
        return m_animationEndedThisFrame;

    if (cond.type == ConditionType::OnTrigger)
        return m_pendingTriggers.count(cond.parameter) > 0;

    auto it = m_paramValues.find(cond.parameter);
    if (it == m_paramValues.end())
    {
        logWarning("AnimationGraph: unknown parameter '{}'", cond.parameter);
        return false;
    }

    float v = it->second;
    switch (cond.type)
    {
        case ConditionType::OnValueEqual:       return v == cond.value;
        case ConditionType::OnValueGreaterThan: return v > cond.value;
        case ConditionType::OnValueLessThan:    return v < cond.value;
        default:                                return false;
    }
}

bool AnimationGraph::evaluateTransitions(const std::vector<Transition*>& transitions)
{
    for (const Transition* t : transitions)
    {
        bool allMet = true;
        for (const Condition& cond : t->conditions)
        {
            if (!evaluateCondition(cond))
            {
                allMet = false;
                break;
            }
        }

        if (allMet)
        {
            transitionTo(t->to, t->blendDuration);
            return true;
        }
    }
    return false;
}

void AnimationGraph::transitionTo(const std::string& stateId, float blendDuration)
{
    const StateNode* state = findState(stateId);
    if (!state)
    {
        logWarning("AnimationGraph: transition target state '{}' not found", stateId);
        return;
    }

    m_currentStateId = stateId;
    m_animationEndedThisFrame = false;

    if (m_owner)
        m_owner->playAnimation(state->animationEntry.name);
}

const StateNode* AnimationGraph::findState(const std::string& id) const
{
    for (const auto& s : m_states)
    {
        if (s.id == id)
            return &s;
    }
    return nullptr;
}

// ---- JSON ----

void AnimationGraph::loadFromJson(const nlohmann::json& j)
{
    m_states.clear();
    m_transitions.clear();
    m_parameters.clear();
    m_paramValues.clear();

    for (const auto& p : j.value("parameters", nlohmann::json::array()))
    {
        Parameter param;
        param.name         = p.at("name").get<std::string>();
        param.type         = paramTypeFromString(p.value("type", "float"));
        param.defaultValue = p.value("default", 0.f);
        addParameter(param);
    }

    for (const auto& n : j.value("nodes", nlohmann::json::array()))
    {
        std::string id = n.at("id").get<std::string>();
        if (id == ANY_STATE_ID)
            continue;

        StateNode node;
        node.id   = id;
        node.loop = n.value("loop", true);

        if (n.contains("animationEntry"))
        {
            const auto& ae = n["animationEntry"];
            node.animationEntry.name          = ae.value("name", "");
            node.animationEntry.playbackSpeed = ae.value("playbackSpeed", 1.f);
        }

        m_states.push_back(std::move(node));
    }

    for (const auto& c : j.value("connections", nlohmann::json::array()))
    {
        Transition t;
        t.from          = c.at("from").get<std::string>();
        t.to            = c.at("to").get<std::string>();
        t.blendDuration = c.value("blendDuration", 0.2f);

        for (const auto& cond : c.value("conditions", nlohmann::json::array()))
        {
            Condition condition;
            condition.type      = condTypeFromString(cond.at("type").get<std::string>());
            condition.parameter = cond.value("parameter", "");
            condition.value     = cond.value("value", 0.f);
            t.conditions.push_back(condition);
        }

        m_transitions.push_back(std::move(t));
    }

    std::string entry = j.value("entryState", "");
    if (!entry.empty())
        setEntryState(entry);
}

nlohmann::json AnimationGraph::saveToJson() const
{
    nlohmann::json j;

    auto& params = j["parameters"] = nlohmann::json::array();
    for (const auto& p : m_parameters)
    {
        params.push_back({
            { "name",    p.name },
            { "type",    paramTypeToString(p.type) },
            { "default", p.defaultValue }
        });
    }

    auto& nodes = j["nodes"] = nlohmann::json::array();
    for (const auto& s : m_states)
    {
        nodes.push_back({
            { "id",   s.id },
            { "loop", s.loop },
            { "animationEntry", {
                { "name",          s.animationEntry.name },
                { "playbackSpeed", s.animationEntry.playbackSpeed }
            }}
        });
    }

    auto& connections = j["connections"] = nlohmann::json::array();
    for (const auto& t : m_transitions)
    {
        nlohmann::json conn = {
            { "from",          t.from },
            { "to",            t.to },
            { "blendDuration", t.blendDuration }
        };

        auto& conds = conn["conditions"] = nlohmann::json::array();
        for (const auto& c : t.conditions)
        {
            nlohmann::json cobj = { { "type", condTypeToString(c.type) } };
            if (c.type != ConditionType::OnAnimationEnd)
            {
                cobj["parameter"] = c.parameter;
                cobj["value"]     = c.value;
            }
            conds.push_back(cobj);
        }

        connections.push_back(conn);
    }

    j["entryState"] = m_entryStateId;
    return j;
}

void AnimationGraph::removeState(size_t index)
{
    if (index >= m_states.size()) 
        return;

    const std::string id = m_states[index].id;
    m_transitions.erase(
        std::remove_if(m_transitions.begin(), m_transitions.end(),
            [&id](const Transition& t) { return t.from == id || t.to == id; }),
        m_transitions.end());
    m_states.erase(m_states.begin() + index);

    if (m_currentStateId == id) 
        m_currentStateId = m_entryStateId;
    if (m_entryStateId == id) 
        m_entryStateId = "";
}