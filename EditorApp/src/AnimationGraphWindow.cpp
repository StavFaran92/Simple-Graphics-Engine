#include "AnimationGraphWindow.h"

#include <unordered_map>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_node_editor.h"
#include "animation/AnimationGraph.h"
#include "animation/Animator.h"

namespace ed = ax::NodeEditor;

// ---- ID scheme -------------------------------------------------------
//  NodeId 1        = any-state
//  NodeId i+2      = state i  (i = 0-based index into getStates())
//  PinId  nodeId + 1000 = output pin of that node
//  PinId  nodeId + 2000 = input  pin of that node
//  LinkId transIdx + 3000 = transition transIdx
// ----------------------------------------------------------------------

static constexpr uint64_t ANY_NODE_ID = 1;
static constexpr uint64_t ANY_OUT_PIN = ANY_NODE_ID + 1000; // 1001

static ed::NodeId nodeIdOf(size_t si)    { return ed::NodeId(si + 2); }
static ed::PinId  outPinOf(size_t si)    { return ed::PinId(si + 2 + 1000); }
static ed::PinId  inPinOf(size_t si)     { return ed::PinId(si + 2 + 2000); }
static ed::LinkId linkIdOf(size_t ti)    { return ed::LinkId(ti + 3000); }

static bool   pinIsOutput(uint64_t p) { return p >= 1001 && p < 2000; }
static size_t outPinToState(uint64_t p){ return (p == ANY_OUT_PIN) ? SIZE_MAX : p - 1002; }
static size_t inPinToState(uint64_t p) { return p - 2002; }

// ---- Condition editor -----------------------------------------------

static const Parameter* findParam(const std::vector<Parameter>& params, const std::string& name)
{
    for (const auto& p : params)
        if (p.name == name) return &p;
    return nullptr;
}

static bool isGlobalCondType(ConditionType t)
{
    return t == ConditionType::OnAnimationEnd;
}

static void displayConditionEditor(Condition& cond, const std::vector<Parameter>& params, int id)
{
    ImGui::PushID(id);

    // --- Row 1: Global vs Parameter ---
    static const char* categoryLabels[] = { "Global", "Parameter" };
    int catIdx = isGlobalCondType(cond.type) ? 0 : 1;
    ImGui::SetNextItemWidth(-1);
    if (ImGui::Combo("##cat", &catIdx, categoryLabels, 2))
    {
        if (catIdx == 0) { cond.type = ConditionType::OnAnimationEnd; cond.parameter = ""; }
        else             { cond.type = ConditionType::OnValueEqual;   cond.parameter = ""; }
    }

    // --- Row 2a: Global condition type ---
    if (catIdx == 0)
    {
        static const char* globalLabels[] = { "On Animation End" };
        int gIdx = 0; // only one global type for now
        ImGui::SetNextItemWidth(-1);
        ImGui::Combo("##globaltype", &gIdx, globalLabels, 1);
        cond.type = ConditionType::OnAnimationEnd;
        ImGui::PopID();
        return;
    }

    // --- Row 2b: Parameter selector ---
    {
        const char* preview = cond.parameter.empty() ? "(none)" : cond.parameter.c_str();
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##param", preview))
        {
            for (const auto& p : params)
            {
                bool sel = (p.name == cond.parameter);
                if (ImGui::Selectable(p.name.c_str(), sel))
                {
                    cond.parameter = p.name;
                    if      (p.type == ParameterType::Trigger) cond.type = ConditionType::OnTrigger;
                    else if (p.type == ParameterType::Bool)    cond.type = ConditionType::OnValueEqual;
                    else                                       cond.type = ConditionType::OnValueEqual;
                }
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    // --- Row 3: Operator/value — driven by selected param type ---
    const Parameter* selParam = findParam(params, cond.parameter);
    if (!selParam) { ImGui::PopID(); return; }

    if (selParam->type == ParameterType::Trigger)
    {
        // Trigger fires on selection alone — no operator or value needed
        cond.type = ConditionType::OnTrigger;
    }
    else if (selParam->type == ParameterType::Bool)
    {
        static const char* boolLabels[] = { "Is True", "Is False" };
        int boolIdx = (cond.value >= 0.5f) ? 0 : 1;
        ImGui::SetNextItemWidth(-1);
        if (ImGui::Combo("##boolval", &boolIdx, boolLabels, 2))
            cond.value = (boolIdx == 0) ? 1.f : 0.f;
        cond.type = ConditionType::OnValueEqual;
    }
    else // Float / Int
    {
        static const char* cmpLabels[] = { "==", ">", "<" };
        int cmpIdx = cond.type == ConditionType::OnValueGreaterThan ? 1
                   : cond.type == ConditionType::OnValueLessThan    ? 2 : 0;
        ImGui::SetNextItemWidth(40);
        if (ImGui::Combo("##cmp", &cmpIdx, cmpLabels, 3))
            cond.type = cmpIdx == 1 ? ConditionType::OnValueGreaterThan
                      : cmpIdx == 2 ? ConditionType::OnValueLessThan
                                    : ConditionType::OnValueEqual;
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
        ImGui::DragFloat("##val", &cond.value, 0.01f);
    }

    ImGui::PopID();
}

// ---- Parameters panel -----------------------------------------------

static void displayParameters(AnimationGraph* graph)
{
    if (!graph) return;
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextUnformatted("Parameters");
    ImGui::Separator();

    static const char* typeLabels[] = { "Float", "Bool", "Int", "Trigger" };
    const auto& params = graph->getParameters();

    for (int i = 0; i < (int)params.size(); ++i)
    {
        Parameter* p = graph->getParameter(i);
        ImGui::PushID(i);

        bool open = ImGui::CollapsingHeader(p->name.c_str());

        if (open)
        {
            ImGui::Indent();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputText("Name", &p->name);

            int typeIdx = (int)p->type;
            ImGui::SetNextItemWidth(-1);
            if (ImGui::Combo("Type", &typeIdx, typeLabels, 4))
                p->type = (ParameterType)typeIdx;

            if (p->type == ParameterType::Bool)
            {
                bool b = p->defaultValue != 0.f;
                if (ImGui::Checkbox("Default", &b)) p->defaultValue = b ? 1.f : 0.f;
            }
            else if (p->type == ParameterType::Trigger)
            {
                if (ImGui::SmallButton("Fire")) graph->trigger(p->name);
            }
            else
            {
                ImGui::SetNextItemWidth(-1);
                ImGui::DragFloat("Default", &p->defaultValue, 0.01f);
            }

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.f));
            if (ImGui::Button("Delete", ImVec2(-1, 0)))
            {
                ImGui::PopStyleColor();
                graph->removeParameter(i--);
                ImGui::Unindent();
                ImGui::PopID();
                continue;
            }
            ImGui::PopStyleColor();

            ImGui::Unindent();
        }

        ImGui::PopID();
    }

    if (ImGui::Button("+ Parameter"))
        graph->addParameter({ "NewParam", ParameterType::Float, 0.f });
}

// ---- Node pool helper -----------------------------------------------

static void addNodeFromEntry(AnimationGraph* graph, const AnimationEntry& entry)
{
    std::string id = entry.name;
    int suffix = 1;
    const auto& states = graph->getStates();
    while (std::any_of(states.begin(), states.end(),
                       [&](const StateNode& s){ return s.id == id; }))
        id = entry.name + "_" + std::to_string(suffix++);

    StateNode node;
    node.id             = id;
    node.animationEntry = entry;
    node.loop           = true;
    graph->addState(node);
}

// ---- Viewer (right panel) -------------------------------------------

static void displayViewer(AnimationGraph* graph, size_t selStateIdx, size_t selTransIdx)
{
    ImGui::TextUnformatted("Viewer");
    ImGui::Separator();

    if (!graph)
    {
        ImGui::TextDisabled("No graph loaded.");
        return;
    }

    if (selStateIdx == SIZE_MAX)
    {
        ImGui::TextDisabled("Select a node to view its properties.");
        return;
    }

    const StateNode* node = graph->getState(selStateIdx);
    if (!node) return;

    ImGui::LabelText("ID",        "%s", node->id.c_str());
    ImGui::LabelText("Animation", "%s", node->animationEntry.name.c_str());
    ImGui::LabelText("Loop",      "%s", node->loop ? "yes" : "no");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextUnformatted("Transitions");
    ImGui::Separator();

    const auto& params      = graph->getParameters();
    const auto& transitions = graph->getTransitions();
    bool any = false;

    for (size_t i = 0; i < transitions.size(); ++i)
    {
        if (transitions[i].from != node->id) continue;
        any = true;

        Transition* t = graph->getTransition(i);
        bool highlight = (selTransIdx == i);
        ImGui::PushID((int)i);

        std::string label = "-> " + t->to;
        if (highlight)
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.5f, 0.8f, 1.f));

        bool open = ImGui::CollapsingHeader(label.c_str());

        if (open)
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

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.f));
            if (ImGui::Button("Delete Transition", ImVec2(-1, 0)))
            {
                if (highlight) ImGui::PopStyleColor(2);
                else           ImGui::PopStyleColor(1);
                graph->removeTransition(i--);
                ImGui::PopID();
                ImGui::Unindent();
                continue;
            }
            ImGui::PopStyleColor();

            ImGui::Unindent();
        }

        if (highlight) ImGui::PopStyleColor();
        ImGui::PopID();
    }

    if (!any)
        ImGui::TextDisabled("No outgoing transitions.");
}

// ---- Static window state --------------------------------------------

static bool               s_open        = false;
static AnimationGraph*    s_graph       = nullptr;
static Animator*          s_animator    = nullptr;
static ed::EditorContext* s_edCtx       = nullptr;
static bool               s_firstFrame  = true;
static size_t             s_selState    = SIZE_MAX;
static size_t             s_selTrans    = SIZE_MAX;
static size_t             s_pendingFrom = SIZE_MAX;
static uint64_t           s_ctxNode     = 0;
static uint64_t           s_ctxLink     = 0;

// Per-node bar width measured last frame (screen pixels) for sizing the top/bottom pin bars.
static std::unordered_map<uint64_t, float>   s_nodeW;
static std::unordered_map<uint64_t, ImVec2>  s_nodeScreenCenter; // node center, screen space

// ---- Public API -----------------------------------------------------

void AnimationGraphWindow::open(Animator* animator)
{
    s_animator    = animator;
    s_graph       = &animator->getAnimationGraph();
    s_open        = true;
    s_firstFrame  = true;
    s_selState    = SIZE_MAX;
    s_selTrans    = SIZE_MAX;
    s_pendingFrom = SIZE_MAX;

    if (!s_edCtx)
    {
        ed::Config cfg;
        cfg.SettingsFile = nullptr; // positions managed via JSON
        s_edCtx = ed::CreateEditor(&cfg);
    }
}

nlohmann::json AnimationGraphWindow::saveToJson()
{
    nlohmann::json j;
    if (!s_graph) return j;

    j["graph"] = s_graph->saveToJson();

    if (s_edCtx)
    {
        ed::SetCurrentEditor(s_edCtx);
        auto& layout = j["layout"];

        const auto& states = s_graph->getStates();
        for (size_t i = 0; i < states.size(); ++i)
        {
            ImVec2 pos = ed::GetNodePosition(nodeIdOf(i));
            layout[states[i].id] = { {"x", pos.x}, {"y", pos.y} };
        }
        ImVec2 anyPos = ed::GetNodePosition(ed::NodeId(ANY_NODE_ID));
        layout[AnimationGraph::ANY_STATE_ID] = { {"x", anyPos.x}, {"y", anyPos.y} };

        ed::SetCurrentEditor(nullptr);
    }
    return j;
}

void AnimationGraphWindow::loadFromJson(const nlohmann::json& j)
{
    if (!s_graph) return;
    if (j.contains("graph"))
        s_graph->loadFromJson(j["graph"]);
    s_firstFrame = true; // positions applied on first frame via SetNodePosition
}

// ---- display() ------------------------------------------------------

void AnimationGraphWindow::display()
{
    if (!s_open) return;

    ImGui::SetNextWindowSize({ 1100, 650 }, ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Animation Graph", &s_open, ImGuiWindowFlags_NoMove))
    {
        ImGui::End();
        if (!s_open && s_edCtx) { ed::DestroyEditor(s_edCtx); s_edCtx = nullptr; }
        return;
    }

    // Title-bar-only drag
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        ImVec2 cp = ImGui::GetIO().MouseClickedPos[ImGuiMouseButton_Left];
        ImVec2 wp = ImGui::GetWindowPos();
        if (cp.y >= wp.y && cp.y <= wp.y + ImGui::GetFrameHeight())
        {
            ImVec2 d = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            ImGui::SetWindowPos(ImVec2(wp.x + d.x, wp.y + d.y));
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }
    }

    const float leftW  = 200.f;
    const float rightW = 270.f;
    const float sp     = ImGui::GetStyle().ItemSpacing.x;
    const float graphW = ImGui::GetContentRegionAvail().x - leftW - rightW - sp * 2.f;
    const float h      = ImGui::GetContentRegionAvail().y;

    // ----------------------------------------------------------------
    // Left panel
    // ----------------------------------------------------------------
    ImGui::BeginChild("##left", ImVec2(leftW, h), true);

    if (s_pendingFrom != SIZE_MAX && s_graph)
    {
        ImGui::TextColored(ImVec4(1.f, 0.8f, 0.f, 1.f), "From: %s",
            s_graph->getStates()[s_pendingFrom].id.c_str());
        ImGui::TextDisabled("Right-click target node");
        if (ImGui::Button("Cancel", ImVec2(-1, 0)))
            s_pendingFrom = SIZE_MAX;
        ImGui::Separator();
    }

    ImGui::TextUnformatted("Add Node");
    ImGui::Separator();
    if (s_animator && !s_animator->getAllAnimations().empty())
    {
        for (const auto& entry : s_animator->getAllAnimations())
            if (ImGui::Button(entry.name.c_str(), ImVec2(-1, 0)))
                addNodeFromEntry(s_graph, entry);
    }
    else
        ImGui::TextDisabled("No animations.");

    displayParameters(s_graph);
    ImGui::EndChild();

    ImGui::SameLine();

    // ----------------------------------------------------------------
    // Graph canvas
    // ----------------------------------------------------------------
    ImGui::BeginChild("##graph", ImVec2(graphW, h));
    ed::SetCurrentEditor(s_edCtx);
    ed::Begin("##ned", ImVec2(0, 0));

    if (s_graph)
    {
        const auto& states      = s_graph->getStates();
        const auto& transitions = s_graph->getTransitions();
        const auto* curState    = s_graph->getCurrentState();
        const std::string entryState    = s_graph->getEntryState();

        // ---- Tree/Sequence style: top input bar, content, bottom output bar ----
        const float rounding = 5.f;
        const float barH     = 14.f;
        const float minW     = 120.f;
        const float minH     = 40;

        // Capture default dark node bg color (used for pin bar fill — matches example)
        const auto pinBg = ed::GetStyle().Colors[ed::StyleColor_NodeBg];
        const ImU32 pinBgCol = IM_COL32((int)(255*pinBg.x), (int)(255*pinBg.y), (int)(255*pinBg.z), 200);

        ed::PushStyleColor(ed::StyleColor_NodeBg,     ImVec4(0.40f, 0.40f, 0.40f, 1.00f)); // light gray, opaque
        ed::PushStyleColor(ed::StyleColor_NodeBorder, ImVec4(0.00f, 0.00f, 0.00f, 1.00f)); // black
        ed::PushStyleColor(ed::StyleColor_PinRect,       ImVec4(0.24f, 0.71f, 1.00f, 0.59f)); // blue highlight
        ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImVec4(0.24f, 0.71f, 1.00f, 0.59f));
        ed::PushStyleVar(ed::StyleVar_NodePadding,     ImVec4(0, 0, 0, 0));
        ed::PushStyleVar(ed::StyleVar_NodeRounding,    rounding);
        ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.f,  1.f));
        ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.f, -1.f));
        ed::PushStyleVar(ed::StyleVar_LinkStrength,    1.f);
        ed::PushStyleVar(ed::StyleVar_PinBorderWidth,  1.f);
        ed::PushStyleVar(ed::StyleVar_PinRadius,       5.f);

        // -- State nodes --
        for (size_t i = 0; i < states.size(); ++i)
        {
            const StateNode& s      = states[i];
            bool active  = curState && curState->id == s.id;
            bool pending = s_pendingFrom == i;
            uint64_t nid = nodeIdOf(i).Get();
            bool isEntryState = entryState == s.id;

            if (isEntryState) ed::PushStyleColor(ed::StyleColor_NodeBg, ImVec4(0.24f, 0.71f, 1.00f, 0.59f));

            // Border tint indicates state: green=active, yellow=pending, default dark
            ImVec4 borderCol = active  ? ImVec4(0.2f, 0.8f, 0.3f, 1.f) :
                               pending ? ImVec4(0.9f, 0.7f, 0.1f, 1.f) :
                                         ImVec4(0.00f, 0.00f, 0.00f, 1.f);
            ed::PushStyleColor(ed::StyleColor_NodeBorder, borderCol);
            ed::BeginNode(nodeIdOf(i));

                // Content — single line, centered X and Y
                ImVec2 cTL = ImGui::GetCursorScreenPos();
                ImGui::BeginGroup();
                    float originX = ImGui::GetCursorPosX();
                    float originY = ImGui::GetCursorPosY();
                    ImGui::Dummy(ImVec2(minW, minH));
                    ImVec2 textSize = ImGui::CalcTextSize(s.id.c_str());
                    ImGui::SetCursorPosX(originX + (minW - textSize.x) * 0.5f);
                    ImGui::SetCursorPosY(originY + (minH - textSize.y) * 0.5f);
                    if (active) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.7f, 0.2f, 1.f));
                    ImGui::TextUnformatted(s.id.c_str());
                    if (active) ImGui::PopStyleColor();
                ImGui::EndGroup();
                ImVec2 cBR = ImGui::GetItemRectMax();

                // Store center in screen space for manual link drawing
                s_nodeScreenCenter[nid] = ImVec2(
                    (cTL.x + cBR.x) * 0.5f,
                    (cTL.y + cBR.y) * 0.5f);

            ed::EndNode();
            ed::PopStyleColor(1);
            if (isEntryState)  ed::PopStyleColor();


            //drawBars(nodeIdOf(i), topTL, topBR, cTL, cBR, botTL, botBR);
        }

        ed::PopStyleColor(4); // NodeBg, NodeBorder, PinRect, PinRectBorder
        ed::PopStyleVar(7);   // NodePadding, NodeRounding, SourceDir, TargetDir, LinkStrength, PinBorderWidth, PinRadius

        // Draw transition lines inside ed::Begin/End so they live in the canvas
        // draw list — zoom, pan and clipping are handled automatically.
        {
            ImDrawList* dl = ImGui::GetWindowDrawList();

            auto getCenterOf = [&](const std::string& id) -> ImVec2
            {
                for (size_t k = 0; k < states.size(); ++k)
                    if (states[k].id == id)
                    {
                        auto it = s_nodeScreenCenter.find(nodeIdOf(k).Get());
                        if (it != s_nodeScreenCenter.end()) return it->second;
                    }
                if (id == AnimationGraph::ANY_STATE_ID)
                {
                    auto it = s_nodeScreenCenter.find(ANY_NODE_ID);
                    if (it != s_nodeScreenCenter.end()) return it->second;
                }
                return ImVec2(-1, -1);
            };

            for (const auto& t : transitions)
            {
                ImVec2 src = getCenterOf(t.from);
                ImVec2 dst = getCenterOf(t.to);
                //if (src.x < 0 || dst.x < 0) continue;

                float dx = dst.x - src.x, dy = dst.y - src.y;
                float len = sqrtf(dx*dx + dy*dy);
                if (len < 1.f) continue;
                float nx = dx / len, ny = dy / len;

                dl->AddLine(src, dst, IM_COL32(180, 180, 180, 200), 1.5f);

                // Arrowhead at 55% of the way (near midpoint, pointing toward dst)
                const float aSize = 9.f;
                ImVec2 tip(src.x + nx * len * 0.55f, src.y + ny * len * 0.55f);
                ImVec2 b1(tip.x - nx*aSize - ny*aSize*0.5f, tip.y - ny*aSize + nx*aSize*0.5f);
                ImVec2 b2(tip.x - nx*aSize + ny*aSize*0.5f, tip.y - ny*aSize - nx*aSize*0.5f);
                dl->AddTriangleFilled(tip, b1, b2, IM_COL32(200, 200, 200, 230));
            }

            // Preview line: source node center → mouse while transition pending
            if (s_pendingFrom != SIZE_MAX && s_graph)
            {
                uint64_t srcNid = nodeIdOf(s_pendingFrom).Get();
                auto it = s_nodeScreenCenter.find(srcNid);
                if (it != s_nodeScreenCenter.end())
                {
                    ImVec2 src = it->second;
                    ImVec2 dst = ImGui::GetMousePos();
                    dl->AddLine(src, dst, IM_COL32(255, 220, 60, 200), 2.f);
                    dl->AddCircleFilled(dst, 5.f, IM_COL32(255, 220, 60, 220));
                }
            }
        }

        // No drag-to-connect — transitions created via right-click menu only

        // -- Deletion (Delete key) --
        if (ed::BeginDelete())
        {
            ed::LinkId delLink;
            while (ed::QueryDeletedLink(&delLink))
            {
                if (ed::AcceptDeletedItem())
                {
                    size_t ti = (size_t)delLink.Get() - 3000;
                    if (ti < s_graph->getTransitions().size())
                        s_graph->removeTransition(ti);
                }
            }
            ed::NodeId delNode;
            while (ed::QueryDeletedNode(&delNode))
            {
                uint64_t nid = delNode.Get();
                if (nid != ANY_NODE_ID)
                {
                    size_t si = nid - 2;
                    if (si < s_graph->getStates().size())
                        s_graph->removeState(si);
                }
                ed::AcceptDeletedItem();
            }
        }
        ed::EndDelete();

        // -- Context menus --
        {
            ed::NodeId ctxNodeId;
            ed::LinkId ctxLinkId;

            ed::Suspend();

            if (ed::ShowNodeContextMenu(&ctxNodeId))
            {
                s_ctxNode = ctxNodeId.Get();
                ImGui::OpenPopup("##nodeCtx");
            }
            else if (ed::ShowLinkContextMenu(&ctxLinkId))
            {
                s_ctxLink = ctxLinkId.Get();
                ImGui::OpenPopup("##linkCtx");
            }
            else if (ed::ShowBackgroundContextMenu())
                ImGui::OpenPopup("##bgCtx");

            // Node context
            if (ImGui::BeginPopup("##nodeCtx"))
            {
                if (s_ctxNode != ANY_NODE_ID)
                {
                    size_t si = s_ctxNode - 2;
                    if (si < s_graph->getStates().size())
                    {
                        if (ImGui::MenuItem("Add Transition"))
                            s_pendingFrom = si;

                        if (ImGui::MenuItem("Make Entry State"))
                            s_graph->setEntryState(s_graph->getState(si)->id);

                        ImGui::Separator();
                        if (ImGui::MenuItem("Delete"))
                            s_graph->removeState(si);
                    }
                }
                ImGui::EndPopup();
            }

            //// Link context
            //if (ImGui::BeginPopup("##linkCtx"))
            //{
            //    size_t ti = s_ctxLink - 3000;
            //    if (ti < s_graph->getTransitions().size())
            //        if (ImGui::MenuItem("Delete Transition"))
            //            s_graph->removeTransition(ti);
            //    ImGui::EndPopup();
            //}

            //// Background context
            //if (ImGui::BeginPopup("##bgCtx"))
            //{
            //    if (s_pendingFrom != SIZE_MAX && ImGui::MenuItem("Cancel Transition"))
            //        s_pendingFrom = SIZE_MAX;
            //    else
            //        ImGui::TextDisabled("(empty)");
            //    ImGui::EndPopup();
            //}

            ed::Resume();
        }

        // -- Selection → complete pending transition or update viewer --
        {
            ed::NodeId selNodes[1];
            int selCount = ed::GetSelectedNodes(selNodes, 1);

            if (selCount > 0)
            {
                uint64_t selectedNid = selNodes[0].Get();
                size_t   selectedSi  = (selectedNid != ANY_NODE_ID && selectedNid >= 2)
                                       ? selectedNid - 2 : SIZE_MAX;

                if (s_pendingFrom != SIZE_MAX && selectedSi != SIZE_MAX
                    && selectedSi != s_pendingFrom)
                {
                    // Complete the pending transition
                    Transition t;
                    t.from          = s_graph->getStates()[s_pendingFrom].id;
                    t.to            = s_graph->getStates()[selectedSi].id;
                    t.blendDuration = 0.2f;
                    s_graph->addTransition(t);
                    s_pendingFrom = SIZE_MAX;
                    ed::ClearSelection();
                }
                else
                {
                    s_selState = selectedSi;
                }
            }
            else
            {
                s_selState = SIZE_MAX;
            }

            ed::LinkId selLinks[1];
            s_selTrans = SIZE_MAX;
            if (ed::GetSelectedLinks(selLinks, 1) > 0)
            {
                uint64_t lid = selLinks[0].Get();
                if (lid >= 3000) s_selTrans = lid - 3000;
            }
        }
    }

    if (s_firstFrame)
    {
        ed::NavigateToContent();
        s_firstFrame = false;
    }

    ed::End();
    ed::SetCurrentEditor(nullptr);

    ImGui::EndChild(); // ##graph

    ImGui::SameLine();

    // ----------------------------------------------------------------
    // Right viewer
    // ----------------------------------------------------------------
    ImGui::BeginChild("##viewer", ImVec2(rightW, h), true);
    displayViewer(s_graph, s_selState, s_selTrans);
    ImGui::EndChild();

    ImGui::End();
}
