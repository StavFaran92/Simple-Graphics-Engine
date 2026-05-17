#include "AnimationGraphWindow.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "GraphEditor.h"
#include "animation/AnimationGraph.h"
#include "animation/Animator.h"

static Animator* s_animator = nullptr;

// ---- Delegate ----

struct AnimGraphDelegate : public GraphEditor::Delegate
{
    AnimationGraph* graph = nullptr;

    static constexpr int TEMPLATE_STATE    = 0;
    static constexpr int TEMPLATE_ANYSTATE = 1;

    std::vector<ImRect> nodeRects;

    // Selection
    size_t selectedNodeIndex = SIZE_MAX;  // GraphEditor node index (for viewer)
    std::vector<bool> selectedNodes;      // parallel to GetNodeCount(), for drag

    void syncRects()
    {
        if (!graph) return;
        const auto& states = graph->getStates();
        while (nodeRects.size() < states.size())
        {
            float x = 50.f + (float)nodeRects.size() * 180.f;
            nodeRects.push_back(ImRect{ {x, 50.f}, {x + 150.f, 100.f} });
        }
        nodeRects.resize(states.size());
    }

    static constexpr size_t ANY_STATE_NODE = 0;
    size_t stateIndexToNode(size_t si) const { return si + 1; }
    size_t nodeToStateIndex(size_t ni) const { return ni - 1; }
    bool   isAnyStateNode(size_t ni)   const { return ni == ANY_STATE_NODE; }

    const std::string& nodeId(size_t ni) const
    {
        if (isAnyStateNode(ni)) return s_anyStateId;
        return graph->getStates()[nodeToStateIndex(ni)].id;
    }

    inline static const std::string s_anyStateId{ AnimationGraph::ANY_STATE_ID };

    // ---- Delegate interface ----

    bool AllowedLink(GraphEditor::NodeIndex from, GraphEditor::NodeIndex to) override
    {
        return !isAnyStateNode(to);
    }

    void SelectNode(GraphEditor::NodeIndex nodeIndex, bool selected) override
    {
        // Complete a pending transition
        if (selected && pendingFrom != SIZE_MAX && nodeIndex != pendingFrom && graph)
        {
            Transition t;
            t.from          = nodeId(pendingFrom);
            t.to            = nodeId(nodeIndex);
            t.blendDuration = 0.2f;
            graph->addTransition(t);
            pendingFrom = SIZE_MAX;
            return;
        }

        // Grow the selection buffer on demand
        if (nodeIndex >= selectedNodes.size())
            selectedNodes.resize(nodeIndex + 1, false);

        selectedNodes[nodeIndex] = selected;

        // Update viewer selection
        if (selected && !isAnyStateNode(nodeIndex))
            selectedNodeIndex = nodeIndex;
        else if (!selected && selectedNodeIndex == nodeIndex)
            selectedNodeIndex = SIZE_MAX;
    }

    void MoveSelectedNodes(const ImVec2 delta) override
    {
        if (!graph) return;
        for (size_t i = 1; i <= graph->getStates().size(); ++i)
        {
            if (i < selectedNodes.size() && selectedNodes[i])
            {
                ImRect& r = nodeRects[nodeToStateIndex(i)];
                r.Min.x += delta.x; r.Min.y += delta.y;
                r.Max.x += delta.x; r.Max.y += delta.y;
            }
        }
    }

    void AddLink(GraphEditor::NodeIndex, GraphEditor::SlotIndex,
                 GraphEditor::NodeIndex, GraphEditor::SlotIndex) override
    {
        // Connections are created via right-click "Make Transition", not port dragging.
    }

    void DelLink(GraphEditor::LinkIndex) override {}

    void CustomDraw(ImDrawList* drawList, ImRect rect, GraphEditor::NodeIndex nodeIndex) override
    {
        if (!graph || isAnyStateNode(nodeIndex)) return;

        const StateNode& s       = graph->getStates()[nodeToStateIndex(nodeIndex)];
        const StateNode* current = graph->getCurrentState();

        ImVec2 textPos = ImVec2(rect.Min.x + 4, rect.Min.y + 2);
        ImU32  col     = (current && current->id == s.id)
            ? IM_COL32(100, 220, 100, 255)
            : IM_COL32(200, 200, 200, 255);

        drawList->AddText(textPos, col, s.animationEntry.name.c_str());

        // Highlight the node that is the pending transition source
        if (nodeIndex == pendingFrom)
            drawList->AddRect(rect.Min, rect.Max, IM_COL32(255, 200, 0, 255), 0.f, 0, 3.f);
    }

    // Pending transition state
    size_t pendingFrom = SIZE_MAX;

    void RightClick(GraphEditor::NodeIndex nodeIndex, GraphEditor::SlotIndex, GraphEditor::SlotIndex) override
    {
        if (nodeIndex != SIZE_MAX && !isAnyStateNode(nodeIndex))
            pendingFrom = nodeIndex;   // right-click a node = mark it as transition source
        else
            pendingFrom = SIZE_MAX;    // right-click canvas = cancel
    }

    // ---- Templates (no ports) ----

    const size_t GetTemplateCount() override { return 2; }

    const GraphEditor::Template GetTemplate(GraphEditor::TemplateIndex index) override
    {
        if (index == TEMPLATE_ANYSTATE)
        {
            return GraphEditor::Template{
                IM_COL32(180, 60,  60,  255),
                IM_COL32(60,  30,  30,  255),
                IM_COL32(80,  40,  40,  255),
                0, nullptr, nullptr,
                0, nullptr, nullptr
            };
        }
        return GraphEditor::Template{
            IM_COL32(50,  90,  160, 255),
            IM_COL32(30,  40,  60,  255),
            IM_COL32(40,  55,  80,  255),
            0, nullptr, nullptr,
            0, nullptr, nullptr
        };
    }

    // ---- Nodes ----

    const size_t GetNodeCount() override
    {
        if (!graph) return 1;
        syncRects();
        return graph->getStates().size() + 1;
    }

    const GraphEditor::Node GetNode(GraphEditor::NodeIndex index) override
    {
        if (isAnyStateNode(index))
        {
            return GraphEditor::Node{
                "Any State", TEMPLATE_ANYSTATE,
                ImRect{{10.f, 10.f}, {160.f, 60.f}}, false
            };
        }
        syncRects();
        const StateNode& s  = graph->getStates()[nodeToStateIndex(index)];
        bool             sel = index < selectedNodes.size() && selectedNodes[index];
        return GraphEditor::Node{
            s.id.c_str(), TEMPLATE_STATE,
            nodeRects[nodeToStateIndex(index)], sel
        };
    }

    // ---- Links ----

    const size_t GetLinkCount() override
    {
        return graph ? graph->getTransitions().size() : 0;
    }

    const GraphEditor::Link GetLink(GraphEditor::LinkIndex index) override
    {
        const Transition& t    = graph->getTransitions()[index];
        const auto&       states = graph->getStates();

        auto findNode = [&](const std::string& id) -> GraphEditor::NodeIndex
        {
            if (id == AnimationGraph::ANY_STATE_ID) return ANY_STATE_NODE;
            for (size_t i = 0; i < states.size(); ++i)
                if (states[i].id == id) return stateIndexToNode(i);
            return SIZE_MAX;
        };

        return GraphEditor::Link{ findNode(t.from), 0, findNode(t.to), 0 };
    }
};

// ---- Viewer ----

static const char* condTypeLabels[] = {
    "On Value Equal", "On Value Greater Than", "On Value Less Than", "On Animation End"
};

static void displayConditionEditor(Condition& cond, const std::vector<Parameter>& params, int id)
{
    ImGui::PushID(id);

    int typeIdx = (int)cond.type;
    ImGui::SetNextItemWidth(-1);
    if (ImGui::Combo("##type", &typeIdx, condTypeLabels, 4))
        cond.type = (ConditionType)typeIdx;

    if (cond.type != ConditionType::OnAnimationEnd)
    {
        // Parameter picker
        const char* preview = cond.parameter.empty() ? "(none)" : cond.parameter.c_str();
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##param", preview))
        {
            for (const auto& p : params)
            {
                bool selected = (p.name == cond.parameter);
                if (ImGui::Selectable(p.name.c_str(), selected))
                    cond.parameter = p.name;
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::SetNextItemWidth(-1);
        ImGui::DragFloat("##val", &cond.value, 0.01f);
    }

    ImGui::PopID();
}

static void addNodeFromEntry(AnimationGraph* graph, const AnimationEntry& entry)
{
    std::string id = entry.name;
    int suffix = 1;
    const auto& states = graph->getStates();
    while (std::any_of(states.begin(), states.end(),
                       [&](const StateNode& s) { return s.id == id; }))
        id = entry.name + "_" + std::to_string(suffix++);

    StateNode node;
    node.id             = id;
    node.animationEntry = entry;
    node.loop           = true;
    graph->addState(node);
}

static void displayParameters(AnimationGraph* graph)
{
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextUnformatted("Parameters");
    ImGui::Separator();

    static const char* typeLabels[] = { "Float", "Bool", "Int" };
    const auto& params = graph->getParameters();

    for (int i = 0; i < (int)params.size(); ++i)
    {
        Parameter* p = graph->getParameter(i);
        ImGui::PushID(i);

        ImGui::SetNextItemWidth(80);
        ImGui::InputText("##name", &p->name);
        ImGui::SameLine();

        int typeIdx = (int)p->type;
        ImGui::SetNextItemWidth(55);
        if (ImGui::Combo("##type", &typeIdx, typeLabels, 3))
            p->type = (ParameterType)typeIdx;
        ImGui::SameLine();

        ImGui::SetNextItemWidth(55);
        if (p->type == ParameterType::Bool)
        {
            bool b = p->defaultValue != 0.f;
            if (ImGui::Checkbox("##val", &b))
                p->defaultValue = b ? 1.f : 0.f;
        }
        else
            ImGui::DragFloat("##val", &p->defaultValue, 0.01f);
        ImGui::SameLine();

        if (ImGui::SmallButton("X"))
            graph->removeParameter(i--);

        ImGui::PopID();
    }

    if (ImGui::Button("+ Parameter"))
        graph->addParameter({ "NewParam", ParameterType::Float, 0.f });
}

static void displayViewer(AnimGraphDelegate& delegate)
{
    ImGui::TextUnformatted("Viewer");
    ImGui::Separator();

    AnimationGraph* graph = delegate.graph;
    if (!graph)
    {
        ImGui::TextDisabled("No graph loaded.");
        return;
    }

    if (delegate.selectedNodeIndex == SIZE_MAX ||
        delegate.isAnyStateNode(delegate.selectedNodeIndex))
    {
        ImGui::TextDisabled("Select a node to view its properties.");
        return;
    }

    size_t si = delegate.nodeToStateIndex(delegate.selectedNodeIndex);
    const StateNode* node = graph->getState(si);
    if (!node) return;

    // ---- Node info ----
    ImGui::LabelText("ID",        "%s", node->id.c_str());
    ImGui::LabelText("Animation", "%s", node->animationEntry.name.c_str());
    ImGui::LabelText("Loop",      "%s", node->loop ? "yes" : "no");

    // ---- Outgoing transitions as collapsible headers ----
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextUnformatted("Transitions");
    ImGui::Separator();

    const auto& params = graph->getParameters();
    const auto& transitions = graph->getTransitions();
    bool anyShown = false;

    for (size_t i = 0; i < transitions.size(); ++i)
    {
        if (transitions[i].from != node->id) continue;
        anyShown = true;

        Transition* t = graph->getTransition(i);
        ImGui::PushID((int)i);

        std::string label = "-> " + t->to;
        if (ImGui::CollapsingHeader(label.c_str()))
        {
            ImGui::Indent();

            ImGui::DragFloat("Blend", &t->blendDuration, 0.01f, 0.f, 5.f);

            ImGui::Spacing();
            ImGui::TextUnformatted("Conditions");
            ImGui::Separator();

            for (int ci = 0; ci < (int)t->conditions.size(); ++ci)
            {
                ImGui::PushID(ci);
                displayConditionEditor(t->conditions[ci], params, ci);
                if (ImGui::SmallButton("Remove"))
                    t->conditions.erase(t->conditions.begin() + ci--);
                ImGui::Separator();
                ImGui::PopID();
            }

            if (ImGui::Button("+ Condition"))
                t->conditions.push_back({});

            ImGui::Unindent();
        }

        ImGui::PopID();
    }

    if (!anyShown)
        ImGui::TextDisabled("No outgoing transitions.");
}

// ---- AnimationGraphWindow ----

static bool                     s_open     = false;
static AnimationGraph*          s_graph    = nullptr;
static AnimGraphDelegate        s_delegate;
static GraphEditor::ViewState   s_viewState;
static GraphEditor::Options     s_options;
static GraphEditor::FitOnScreen s_fit = GraphEditor::Fit_AllNodes;

void AnimationGraphWindow::open(AnimationGraph* graph, Animator* animator)
{
    s_graph                      = graph;
    s_animator                   = animator;
    s_delegate.graph             = graph;
    s_delegate.selectedNodeIndex    = SIZE_MAX;
    s_delegate.pendingFrom          = SIZE_MAX;
    s_delegate.selectedNodes.clear();
    s_delegate.nodeRects.clear();
    s_open                          = true;
    s_fit                           = GraphEditor::Fit_AllNodes;
    s_options.mNodeSlotRadius       = 0.f;
}

nlohmann::json AnimationGraphWindow::saveToJson()
{
    nlohmann::json j;
    if (!s_graph) return j;

    j["graph"] = s_graph->saveToJson();

    s_delegate.syncRects();
    const auto& states = s_graph->getStates();
    auto& layout = j["layout"];
    for (size_t i = 0; i < states.size(); ++i)
    {
        const ImRect& r = s_delegate.nodeRects[i];
        layout[states[i].id] = {
            { "x", r.Min.x }, { "y", r.Min.y },
            { "w", r.Max.x - r.Min.x }, { "h", r.Max.y - r.Min.y }
        };
    }
    return j;
}

void AnimationGraphWindow::loadFromJson(const nlohmann::json& j)
{
    if (!s_graph) return;

    if (j.contains("graph"))
        s_graph->loadFromJson(j["graph"]);

    s_delegate.nodeRects.clear();
    const auto& states = s_graph->getStates();

    if (j.contains("layout"))
    {
        const auto& layout = j["layout"];
        s_delegate.nodeRects.resize(states.size());
        for (size_t i = 0; i < states.size(); ++i)
        {
            const std::string& id = states[i].id;
            if (!layout.contains(id)) continue;
            const auto& e = layout[id];
            float x = e.value("x", 50.f + (float)i * 180.f);
            float y = e.value("y", 50.f);
            float w = e.value("w", 150.f);
            float h = e.value("h", 80.f);
            s_delegate.nodeRects[i] = ImRect{ {x, y}, {x + w, y + h} };
        }
    }

    s_fit = GraphEditor::Fit_AllNodes;
}

void AnimationGraphWindow::display()
{
    if (!s_open) return;

    ImGui::SetNextWindowSize({1000, 600}, ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Animation Graph", &s_open, ImGuiWindowFlags_NoMove))
    {
        ImGui::End();
        return;
    }

    // Title-bar-only dragging
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        ImVec2 clickPos = ImGui::GetIO().MouseClickedPos[ImGuiMouseButton_Left];
        ImVec2 winPos   = ImGui::GetWindowPos();
        if (clickPos.y >= winPos.y && clickPos.y <= winPos.y + ImGui::GetFrameHeight())
        {
            ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            ImGui::SetWindowPos(ImVec2(winPos.x + delta.x, winPos.y + delta.y));
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }
    }

    const float leftWidth  = 200.f;
    const float rightWidth = 270.f;
    const float spacing    = ImGui::GetStyle().ItemSpacing.x;
    const float graphWidth = ImGui::GetContentRegionAvail().x - leftWidth - rightWidth - spacing * 2.f;
    const float height     = ImGui::GetContentRegionAvail().y;

    // Left panel: node pool + parameters
    ImGui::BeginChild("##left", ImVec2(leftWidth, height), true);

    // Pending transition UI
    if (s_delegate.pendingFrom != SIZE_MAX)
    {
        ImGui::TextColored(ImVec4(1.f, 0.8f, 0.f, 1.f), "From: %s",
            s_delegate.nodeId(s_delegate.pendingFrom).c_str());
        ImGui::TextDisabled("Click target node");
        if (ImGui::Button("Cancel", ImVec2(-1, 0)))
            s_delegate.pendingFrom = SIZE_MAX;
        ImGui::Separator();
    }

    ImGui::TextUnformatted("Add Node");
    ImGui::Separator();
    if (s_animator && !s_animator->getAllAnimations().empty())
    {
        for (const auto& entry : s_animator->getAllAnimations())
        {
            if (ImGui::Button(entry.name.c_str(), ImVec2(-1, 0)))
                addNodeFromEntry(s_graph, entry);
        }
    }
    else
        ImGui::TextDisabled("No animations.");

    displayParameters(s_graph);

    ImGui::EndChild();

    ImGui::SameLine();

    // Centre: graph canvas
    ImGui::BeginChild("##graph", ImVec2(graphWidth, height));

    GraphEditor::Show(s_delegate, s_options, s_viewState, true, &s_fit);

    ImGui::EndChild();

    ImGui::SameLine();

    // Right: node / transition viewer
    ImGui::BeginChild("##viewer", ImVec2(rightWidth, height), true);
    displayViewer(s_delegate);
    ImGui::EndChild();

    ImGui::End();
}
