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
        const char* preview = cond.parameter.empty() ? "(none)" : cond.parameter.c_str();
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##param", preview))
        {
            for (const auto& p : params)
            {
                bool sel = (p.name == cond.parameter);
                if (ImGui::Selectable(p.name.c_str(), sel))
                    cond.parameter = p.name;
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
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
        ImGui::SetNextItemWidth(50);
        if (ImGui::Combo("##type", &typeIdx, typeLabels, 3))
            p->type = (ParameterType)typeIdx;
        ImGui::SameLine();

        ImGui::SetNextItemWidth(50);
        if (p->type == ParameterType::Bool)
        {
            bool b = p->defaultValue != 0.f;
            if (ImGui::Checkbox("##val", &b)) p->defaultValue = b ? 1.f : 0.f;
        }
        else
            ImGui::DragFloat("##val", &p->defaultValue, 0.01f);
        ImGui::SameLine();

        if (ImGui::SmallButton("X")) { graph->removeParameter(i--); ImGui::PopID(); continue; }
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

void AnimationGraphWindow::open(AnimationGraph* graph, Animator* animator)
{
    s_graph       = graph;
    s_animator    = animator;
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
        ed::PushStyleVar(ed::StyleVar_LinkStrength,    0.f);
        ed::PushStyleVar(ed::StyleVar_PinBorderWidth,  1.f);
        ed::PushStyleVar(ed::StyleVar_PinRadius,       5.f);

        //auto drawBars = [&](ed::NodeId nid,
        //                    ImVec2 topTL, ImVec2 topBR,
        //                    ImVec2 cTL,   ImVec2 cBR,
        //                    ImVec2 botTL, ImVec2 botBR)
        //{
        //    auto* dl = ed::GetNodeBackgroundDrawList(nid);
        //    // Top input bar — dark gray, rounded bottom corners
        //    ImVec2 topTL1(topTL.x, topTL.y + 1);
        //    dl->AddRectFilled(topTL1, topBR, pinBgCol, rounding, ImDrawFlags_RoundCornersBottom);
        //    dl->AddRect      (topTL1, topBR, pinBgCol, rounding, ImDrawFlags_RoundCornersBottom);
        //    // Content area — dark blue
        //    dl->AddRectFilled(cTL, cBR, IM_COL32(24, 64, 128, 200));
        //    dl->AddRect      (cTL, cBR, IM_COL32(48, 128, 255, 100));
        //    // Bottom output bar — dark gray, rounded top corners
        //    ImVec2 botBR1(botBR.x, botBR.y - 1);
        //    dl->AddRectFilled(botTL, botBR1, pinBgCol, rounding, ImDrawFlags_RoundCornersTop);
        //    dl->AddRect      (botTL, botBR1, pinBgCol, rounding, ImDrawFlags_RoundCornersTop);
        //};

        // -- Any-state node (content + bottom output bar only) --
        {
            float w = std::max(minW, s_nodeW.count(ANY_NODE_ID) ? s_nodeW[ANY_NODE_ID] : minW);
            ed::PushStyleColor(ed::StyleColor_NodeBg,    ImVec4(0.35f, 0.12f, 0.12f, 0.9f));
            ed::PushStyleColor(ed::StyleColor_NodeBorder, ImVec4(0.5f,  0.1f,  0.1f,  1.f));
            ed::BeginNode(ed::NodeId(ANY_NODE_ID));

                ImVec2 cTL = ImGui::GetCursorScreenPos();
                ImGui::BeginGroup();
                    ImGui::Dummy(ImVec2(w, 0));
                    ImGui::TextUnformatted("Any State");
                ImGui::EndGroup();
                ImVec2 cBR = ImGui::GetItemRectMax();
                s_nodeW[ANY_NODE_ID] = cBR.x - cTL.x;
                s_nodeScreenCenter[ANY_NODE_ID] = ImVec2((cTL.x + cBR.x) * 0.5f, (cTL.y + cBR.y) * 0.5f);

                ImVec2 botTL = ImGui::GetCursorScreenPos();
                ed::BeginPin(ed::PinId(ANY_OUT_PIN), ed::PinKind::Output);
#if IMGUI_VERSION_NUM > 18101
                    ed::PushStyleVar(ed::StyleVar_PinCorners, (float)ImDrawFlags_RoundCornersTop);
#else
                    ed::PushStyleVar(ed::StyleVar_PinCorners, 3.f);
#endif
                    ImGui::Dummy(ImVec2(w, barH));
                    ImVec2 botBR = ImGui::GetItemRectMax();
                    ed::PinPivotRect(botTL, botBR);
                    ed::PinRect(botTL, botBR);
                    ed::PopStyleVar();
                ed::EndPin();

            ed::EndNode();
            ed::PopStyleColor(2);

            auto* dl = ed::GetNodeBackgroundDrawList(ed::NodeId(ANY_NODE_ID));
            ImVec2 anyBotBR1(botBR.x, botBR.y - 1);
            dl->AddRectFilled(cTL,   cBR,   IM_COL32(80, 24, 24, 200));
            dl->AddRectFilled(botTL, anyBotBR1, IM_COL32(180, 60, 60, 200), rounding, ImDrawFlags_RoundCornersTop);
            dl->AddRect      (botTL, anyBotBR1, IM_COL32(180, 60, 60, 200), rounding, ImDrawFlags_RoundCornersTop);
        }

        // -- State nodes --
        for (size_t i = 0; i < states.size(); ++i)
        {
            const StateNode& s      = states[i];
            bool active  = curState && curState->id == s.id;
            bool pending = s_pendingFrom == i;
            uint64_t nid = nodeIdOf(i).Get();

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

            //drawBars(nodeIdOf(i), topTL, topBR, cTL, cBR, botTL, botBR);
        }

        ed::PopStyleColor(4); // NodeBg, NodeBorder, PinRect, PinRectBorder
        ed::PopStyleVar(7);   // NodePadding, NodeRounding, SourceDir, TargetDir, LinkStrength, PinBorderWidth, PinRadius

        // Links are drawn manually after ed::End() using stored screen centers

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

                        ImGui::Separator();
                        if (ImGui::MenuItem("Delete"))
                            s_graph->removeState(si);
                    }
                }
                ImGui::EndPopup();
            }

            // Link context
            if (ImGui::BeginPopup("##linkCtx"))
            {
                size_t ti = s_ctxLink - 3000;
                if (ti < s_graph->getTransitions().size())
                    if (ImGui::MenuItem("Delete Transition"))
                        s_graph->removeTransition(ti);
                ImGui::EndPopup();
            }

            // Background context
            if (ImGui::BeginPopup("##bgCtx"))
            {
                if (s_pendingFrom != SIZE_MAX && ImGui::MenuItem("Cancel Transition"))
                    s_pendingFrom = SIZE_MAX;
                else
                    ImGui::TextDisabled("(empty)");
                ImGui::EndPopup();
            }

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

    // Draw transition lines + arrowheads manually (foreground = on top of editor canvas)
    if (s_graph)
    {
        ImDrawList* dl = ImGui::GetForegroundDrawList();
        const auto& states      = s_graph->getStates();
        const auto& transitions = s_graph->getTransitions();

        auto getCenterOf = [&](const std::string& id) -> ImVec2
        {
            for (size_t i = 0; i < states.size(); ++i)
                if (states[i].id == id)
                {
                    auto it = s_nodeScreenCenter.find(nodeIdOf(i).Get());
                    if (it != s_nodeScreenCenter.end()) return it->second;
                }
            // any-state: use its stored center too
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

            // Line
            dl->AddLine(src, dst, IM_COL32(180, 180, 180, 200), 1.5f);

            // Arrowhead at midpoint
            const float aSize = 9.f;
            ImVec2 tip(src.x + nx * len * 0.55f, src.y + ny * len * 0.55f);
            ImVec2 b1(tip.x - nx*aSize - ny*aSize*0.5f, tip.y - ny*aSize + nx*aSize*0.5f);
            ImVec2 b2(tip.x - nx*aSize + ny*aSize*0.5f, tip.y - ny*aSize - nx*aSize*0.5f);
            dl->AddTriangleFilled(tip, b1, b2, IM_COL32(200, 200, 200, 230));
        }
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
            ImDrawList* dl = ImGui::GetForegroundDrawList();
            dl->AddLine(src, dst, IM_COL32(255, 220, 60, 200), 2.f);
            dl->AddCircleFilled(dst, 5.f, IM_COL32(255, 220, 60, 220));
        }
    }

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
