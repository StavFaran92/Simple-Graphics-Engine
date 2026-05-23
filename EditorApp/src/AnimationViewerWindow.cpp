#include "AnimationViewerWindow.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "ImSequencer.h"

#include "AnimationViewRenderer.h"
#include "animation/Animator.h"
#include "animation/Animation.h"
#include "component/MeshRendererComponent.h"
#include "runtime/Entity.h"

static int                  s_selEntry = 0;
static AnimationEntry* s_selectedAnimation = nullptr;

// ---- ImSequencer adapter ------------------------------------------------

struct AnimSequence : public ImSequencer::SequenceInterface
{
    Animator* animator = nullptr;
    int frameMin = 0;
    int frameMax = 100;

    struct Item { 
        int start; 
        int end; 
    };
    Item currentItem;

    void rebuild()
    {
        if (!animator) 
            return;
        int len = 0;
        if (s_selectedAnimation && !s_selectedAnimation->animation.isEmpty() && !s_selectedAnimation->animation.resource().isEmpty())
        {
            float dur = s_selectedAnimation->animation.resource()->getDuration();
            float tps = s_selectedAnimation->animation.resource()->getTicksPerSecond();
            len = tps > 0.f ? (int)(dur / tps * 30.f) : 0;
        }
        currentItem = { 0, len };
        frameMax = currentItem.end;
    }

    int  GetFrameMin() const override { return 0; }
    int  GetFrameMax() const override {
        return frameMax;
    }
    int  GetItemCount() const override {
        return 1;
    }

    const char* GetItemLabel(int /*index*/) const override
    {
        if (!animator || s_selEntry < 0) return "";
        const auto& anims = animator->getAllAnimations();
        if (s_selEntry >= (int)anims.size()) return "";
        return anims[s_selEntry].name.c_str();
    }

    void Get(int /*index*/, int** start, int** end, int* type, unsigned int* color) override
    {
        if (start) *start = &currentItem.start;
        if (end)   *end = &currentItem.end;
        if (type)  *type  = 0;
        if (color) *color = 0xFF4488AA;
    }

    Item GetItem(int index) const
    {
        return currentItem;
    }

    int lastClickedMarkerFrame = -1;
    int pendingRightClickFrame = -1;
    bool wantsOpenPopup = false;

    void addMarker(int itemIndex, int frame)
    {
        s_selectedAnimation->triggers.push_back({ frame });
    }

    void CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clipping_rect) override
    {
        int fMin = GetFrameMin();
        int fMax = GetFrameMax();
        if (fMax <= fMin) return;

        float fw = rc.GetWidth() / float(fMax - fMin + 1);
        ImGuiIO& io = ImGui::GetIO();

        if (ImRect(clipping_rect.Min, clipping_rect.Max).Contains(io.MousePos) && io.MouseClicked[1])
        {
            int f = fMin + (int)((io.MousePos.x - rc.Min.x) / fw);
            pendingRightClickFrame = std::max(fMin, std::min(fMax, f));
            wantsOpenPopup = true;
        }

        draw_list->PushClipRect(clipping_rect.Min, clipping_rect.Max, true);
        for (auto& m : s_selectedAnimation->triggers)
        {
            float x = rc.Min.x + (m.frameID - fMin) * fw;
            ImVec2 p1(x + 1.f, rc.Min.y + 2.f);
            ImVec2 p2(x + fw - 1.f, rc.Max.y - 2.f);
            bool hovered = ImRect(p1, p2).Contains(io.MousePos);
            draw_list->AddRectFilled(p1, p2, hovered ? 0xFFFFCC00 : 0xFFFF8800, 2);
            if (hovered && io.MouseClicked[0])
                lastClickedMarkerFrame = m.frameID;
        }
        draw_list->PopClipRect();
    }
};

// ---- Static state -------------------------------------------------------

static bool                 s_open         = false;
static Entity               s_entity       = Entity::EmptyEntity;
static Animator*            s_animator     = nullptr;

static MeshRendererComponent*        s_meshRenderer = nullptr;

static bool                 s_playing      = false;
static int                  s_currentFrame = 0;
static float s_elapsedTime = 0;
static bool                 s_expanded     = true;
static int                  s_firstFrame   = 0;
static AnimSequence         s_seq;

// ---- Helpers ------------------------------------------------------------

static const AnimationEntry* selectedEntry()
{
    if (!s_animator || s_selEntry < 0) return nullptr;
    const auto& anims = s_animator->getAllAnimations();
    if (s_selEntry >= (int)anims.size()) return nullptr;
    return &anims[s_selEntry];
}

static float currentTimeTicks()
{
    auto& animation = selectedEntry()->animation;
    if (animation.isEmpty() || animation.resource().isEmpty()) return 0.f;
    float tps = animation.resource()->getTicksPerSecond();
    return (s_currentFrame / 30.f) * tps;
}

// ---- Sub-panels ---------------------------------------------------------

static void displayLeftPanel(float w, float h)
{
    ImGui::BeginChild("##av_left", ImVec2(w, h), true);
    ImGui::LabelText("", "Animations");

    if (!s_animator) { ImGui::TextDisabled("No animator."); ImGui::EndChild(); return; }

    const auto& anims = s_animator->getAllAnimations();
    for (int i = 0; i < (int)anims.size(); ++i)
    {
        ImGui::PushID(i);
        bool selected = (i == s_selEntry);
        if (ImGui::Selectable(anims[i].name.c_str(), selected))
        {
            s_selEntry     = i;
            s_selectedAnimation = s_animator->getAnimation(s_selEntry);
            s_currentFrame = 0;
            s_elapsedTime = 0;
            s_playing      = false;
            s_seq.rebuild();
        }
        ImGui::PopID();
    }

    ImGui::EndChild();
}

static void displayRightPanel(float w, float h)
{
    ImGui::BeginChild("##av_right", ImVec2(w, h), true);
    ImGui::LabelText("", "Properties");

    const AnimationEntry* e = selectedEntry();
    if (!e) { ImGui::TextDisabled("Select an animation."); ImGui::EndChild(); return; }

    ImGui::LabelText("Name", "%s", e->name.c_str());

    if (!e->animation.isEmpty() && !e->animation.resource().isEmpty())
    {
        auto res   = e->animation.resource();
        float dur  = res->getDuration();
        float tps  = res->getTicksPerSecond();
        float secs = tps > 0.f ? dur / tps : 0.f;
        ImGui::LabelText("Duration",  "%.2f ticks", dur);
        ImGui::LabelText("Ticks/sec", "%.1f",       tps);
        ImGui::LabelText("Length",    "%.2f s",      secs);
    }
    else
    {
        ImGui::TextDisabled("No animation asset loaded.");
    }

    ImGui::Spacing();
    ImGui::LabelText("", "Playback");

    float speed = e->playbackSpeed;
    if (ImGui::DragFloat("Speed", &speed, 0.01f, 0.f, 10.f))
        s_animator->getAnimation(e->name)->playbackSpeed = speed;

    ImGui::EndChild();
}

static void displayViewport(float w, float h)
{
    ImGui::BeginChild("##av_view", ImVec2(w, h), true);

    ImVec2 avail = ImGui::GetContentRegionAvail();
    uint32_t iw = (uint32_t)avail.x;
    uint32_t ih = (uint32_t)avail.y;

    if (iw > 0 && ih > 0)
    {
        static AnimationViewRenderer s_renderer;

        s_renderer.resize(iw, ih);
        s_renderer.render(selectedEntry()->animation, currentTimeTicks(), s_meshRenderer);

        uint32_t texID = s_renderer.getColorTextureID();
        if (texID)
            ImGui::Image(reinterpret_cast<ImTextureID>(texID), avail, ImVec2(0,1), ImVec2(1,0));
        else
            ImGui::TextDisabled("No render output.");
    }

    ImGui::EndChild();
}

static void displayTimeline()
{
    if (ImGui::Button(s_playing ? "  ||  " : "  >  ")) 
        s_playing = !s_playing;
    ImGui::SameLine();
    if (ImGui::Button(" |< ")) 
    { 
        s_currentFrame = 0; 
        s_elapsedTime = 0;
        s_playing = false; 
    }
    ImGui::SameLine();
    ImGui::Text("Frame %d / %d", s_currentFrame, s_seq.GetItem(s_selEntry).end);

    int frameBefore = s_currentFrame;
    int seqSel = (s_selEntry >= 0) ? 0 : -1;
    ImSequencer::Sequencer(
        &s_seq,
        &s_currentFrame,
        NULL,
        &seqSel,
        &s_firstFrame,
        ImSequencer::SEQUENCER_CHANGE_FRAME
    );
    if (s_currentFrame != frameBefore)
        s_elapsedTime = (float)s_currentFrame;

    if (s_currentFrame >= s_seq.GetItem(s_selEntry).end)
        s_currentFrame = s_seq.GetItem(s_selEntry).end;

if (s_seq.lastClickedMarkerFrame != -1)
    {
        ImGui::SetTooltip("Clicked marker at frame %d", s_seq.lastClickedMarkerFrame); // placeholder
        s_seq.lastClickedMarkerFrame = -1;
    }

    if (s_seq.wantsOpenPopup)
    {
        ImGui::OpenPopup("##timeline_ctx");
        s_seq.wantsOpenPopup = false;
    }

    if (ImGui::BeginPopup("##timeline_ctx"))
    {
        ImGui::Text("Frame %d", s_seq.pendingRightClickFrame);
        ImGui::Separator();
        if (ImGui::MenuItem("Add Trigger"))
            s_seq.addMarker(s_selEntry, s_seq.pendingRightClickFrame);
        ImGui::EndPopup();
    }
    else
        s_seq.pendingRightClickFrame = -1;
}

// ---- Public API ---------------------------------------------------------

void AnimationViewerWindow::open(Entity entity, AnimationAssetRef animation)
{
    s_entity       = entity;
    s_open         = true;
    s_currentFrame = 0;
    s_playing      = false;
    s_firstFrame   = 0;
    s_expanded     = true;

    s_animator = entity.tryGetComponent<Animator>();

    // Find the selected entry index matching the animation
    s_selEntry = -1;
    if (s_animator)
    {
        const auto& anims = s_animator->getAllAnimations();
        for (int i = 0; i < (int)anims.size(); ++i)
        {
            if (!anims[i].animation.isEmpty() &&
                anims[i].animation.getUID() == animation.getUID())
            {
                s_selEntry = i;
                break;
            }
        }
    }

    // Get model from MeshRendererComponent if present
    s_meshRenderer = entity.tryGetComponent<MeshRendererComponent>();

    s_seq.animator = s_animator;
    s_seq.rebuild();
}

void AnimationViewerWindow::display()
{
    if (!s_open) return;

    ImGui::SetNextWindowSize({ 1100, 700 }, ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Animation Viewer", &s_open))
    {
        ImGui::End();
        return;
    }

    // Advance playhead
    if (s_playing)
    {
        float fps = 30.f;
        const AnimationEntry* e = selectedEntry();
        float duration = 0;
        if (e)
        {
            duration = e->animation.resource()->getDuration();
            fps *= e->playbackSpeed;
        }
        s_elapsedTime += ImGui::GetIO().DeltaTime * fps;
        s_currentFrame = (int)s_elapsedTime;
        if (s_currentFrame > duration)
        {
            s_elapsedTime = 0;
            s_currentFrame = 0;
        }
    }

    const float leftW    = 180.f;
    const float rightW   = 200.f;
    const float sp       = ImGui::GetStyle().ItemSpacing.x;
    const float totalH   = ImGui::GetContentRegionAvail().y;
    const float seqH     = 75.f;
    const float contentH = totalH - seqH - sp * 2.f;
    const float viewW    = ImGui::GetContentRegionAvail().x - leftW - rightW - sp * 2.f;

    ImGui::BeginGroup();
    displayLeftPanel(leftW, contentH);
    ImGui::SameLine();
    displayViewport(viewW, contentH);
    ImGui::SameLine();
    displayRightPanel(rightW, contentH);
    ImGui::EndGroup();

    displayTimeline();

    ImGui::End();
}
