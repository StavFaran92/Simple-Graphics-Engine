#include "tests/AnimationGraphTests.h"
#include "animation/AnimationGraph.h"
#include "core/Logger.h"

#include <cassert>

static StateNode makeState(const std::string& id, const std::string& animName, bool loop = true)
{
    StateNode s;
    s.id = id;
    s.animationEntry.name = animName;
    s.loop = loop;
    return s;
}

static Transition makeTransition(const std::string& from, const std::string& to,
                                  float blend, std::vector<Condition> conditions)
{
    Transition t;
    t.from = from;
    t.to = to;
    t.blendDuration = blend;
    t.conditions = std::move(conditions);
    return t;
}

static void testBasicTransition()
{
    AnimationGraph graph;
    graph.addState(makeState("idle", "Idle"));
    graph.addState(makeState("walk", "Walk"));
    graph.addParameter({ "speed", ParameterType::Float, 0.f });
    graph.addTransition(makeTransition("idle", "walk", 0.2f,
        {{ ConditionType::OnValueGreaterThan, "speed", 0.1f }}));
    graph.setEntryState("idle");

    assert(graph.getCurrentState()->id == "idle");

    graph.update(0.016f);
    assert(graph.getCurrentState()->id == "idle"); // speed == 0, no fire

    graph.setFloat("speed", 1.f);
    graph.update(0.016f);
    assert(graph.getCurrentState()->id == "walk");

    logInfo("  testBasicTransition: PASSED");
}

static void testAnimationEndTransition()
{
    AnimationGraph graph;
    graph.addState(makeState("attack", "Attack", false));
    graph.addState(makeState("idle",   "Idle",   true));
    graph.addTransition(makeTransition("attack", "idle", 0.1f,
        {{ ConditionType::OnAnimationEnd }}));
    graph.setEntryState("attack");

    graph.update(0.016f);
    assert(graph.getCurrentState()->id == "attack"); // no end signal yet

    graph.onAnimationEnd();
    graph.update(0.016f);
    assert(graph.getCurrentState()->id == "idle");

    logInfo("  testAnimationEndTransition: PASSED");
}

static void testAnyStateTransition()
{
    AnimationGraph graph;
    graph.addState(makeState("idle",  "Idle"));
    graph.addState(makeState("walk",  "Walk"));
    graph.addState(makeState("death", "Death"));
    graph.addParameter({ "isDead", ParameterType::Bool, 0.f });
    graph.addTransition(makeTransition(AnimationGraph::ANY_STATE_ID, "death", 0.1f,
        {{ ConditionType::OnValueEqual, "isDead", 1.f }}));
    graph.setEntryState("walk");

    graph.setBool("isDead", true);
    graph.update(0.016f);
    assert(graph.getCurrentState()->id == "death");

    logInfo("  testAnyStateTransition: PASSED");
}

static void testAndConditions()
{
    AnimationGraph graph;
    graph.addState(makeState("idle", "Idle"));
    graph.addState(makeState("walk", "Walk"));
    graph.addParameter({ "speed",      ParameterType::Float, 0.f });
    graph.addParameter({ "isGrounded", ParameterType::Bool,  0.f });
    graph.addTransition(makeTransition("idle", "walk", 0.2f, {
        { ConditionType::OnValueGreaterThan, "speed",      0.1f },
        { ConditionType::OnValueEqual,       "isGrounded", 1.f  }
    }));
    graph.setEntryState("idle");

    graph.setFloat("speed", 1.f);
    graph.update(0.016f);
    assert(graph.getCurrentState()->id == "idle"); // isGrounded not set yet

    graph.setBool("isGrounded", true);
    graph.update(0.016f);
    assert(graph.getCurrentState()->id == "walk");

    logInfo("  testAndConditions: PASSED");
}

static void testJsonRoundtrip()
{
    AnimationGraph original;
    original.addState(makeState("idle", "Idle"));
    original.addState(makeState("walk", "Walk"));
    original.addParameter({ "speed", ParameterType::Float, 0.f });
    original.addTransition(makeTransition("idle", "walk", 0.25f,
        {{ ConditionType::OnValueGreaterThan, "speed", 0.5f }}));
    original.setEntryState("idle");

    nlohmann::json j = original.saveToJson();

    AnimationGraph loaded;
    loaded.loadFromJson(j);

    assert(loaded.getCurrentState() != nullptr);
    assert(loaded.getCurrentState()->id == "idle");

    loaded.setFloat("speed", 1.f);
    loaded.update(0.016f);
    assert(loaded.getCurrentState()->id == "walk");

    logInfo("  testJsonRoundtrip: PASSED");
}

void runAnimationGraphTests()
{
    logInfo("--- AnimationGraph ---");
    testBasicTransition();
    testAnimationEndTransition();
    testAnyStateTransition();
    testAndConditions();
    testJsonRoundtrip();
}
