#include "AnimationViewerWindow.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "ImSequencer.h"

#include "AnimationViewRenderer.h"
#include "animation/Animator.h"
#include "animation/Animation.h"
#include "component/MeshRendererComponent.h"
#include "runtime/Entity.h"

static int                  s_selEntry = -1;

// ---- ImSequencer adapter ------------------------------------------------

struct AnimSequence : public ImSequencer::SequenceInterface
{
    Animator* animator = nullptr;

    struct Item { int start; int end; };
    std::vector<Item> items;

    void rebuild()
    {
        items.clear();
        if (!animator) return;
        for (const auto& e : animator->getAllAnimations())
        {
            int len = 0;
            if (!e.animation.isEmpty() && !e.animation.resource().isEmpty())
            {
                float dur = e.animation.resource()->getDuration();
                float tps = e.animation.resource()->getTicksPerSecond();
                len = tps > 0.f ? (int)(dur / tps * 30.f) : 0;
            }
            items.push_back({ 0, len });
        }
    }

    int  GetFrameMin() const override { return GetItem(s_selEntry).start; }
    int  GetFrameMax() const override { return GetItem(s_selEntry).end; }
    int  GetItemCount() const override { return (int)items.size(); }

    const char* GetItemLabel(int index) const override
    {
        if (!animator) return "";
        const auto& anims = animator->getAllAnimations();
        if (index < 0 || index >= (int)anims.size()) return "";
        return anims[index].name.c_str();
    }

    void Get(int index, int** start, int** end, int* type, unsigned int* color) override
    {
        if (index < 0 || index >= (int)items.size()) return;
        if (start) *start = &items[index].start;
        if (end)   *end   = &items[index].end;
        if (type)  *type  = 0;
        if (color) *color = 0xFF4488AA;
    }

    Item GetItem(int index) const
    {
        if (index < 0 || index >= (int)items.size()) return {};

        return items.at(index);
    }
};

// ---- Static state -------------------------------------------------------

static bool                 s_open         = false;
static Entity               s_entity       = Entity::EmptyEntity;
static Animator*            s_animator     = nullptr;
static AnimationAssetRef    s_animation;
static MeshRendererComponent*        s_meshRenderer = nullptr;

static bool                 s_playing      = false;
static int                  s_currentFrame = 0;
static float s_elapsedTime = 0;
static bool                 s_expanded     = true;
static int                  s_firstFrame   = 0;
static AnimSequence         s_seq;

// ---- Helpers ------------------------------------------------------------

static float currentTimeTicks()
{
    if (s_animation.isEmpty() || s_animation.resource().isEmpty()) return 0.f;
    float tps = s_animation.resource()->getTicksPerSecond();
    return (s_currentFrame / 30.f) * tps;
}

static const AnimationEntry* selectedEntry()
{
    if (!s_animator || s_selEntry < 0) return nullptr;
    const auto& anims = s_animator->getAllAnimations();
    if (s_selEntry >= (int)anims.size()) return nullptr;
    return &anims[s_selEntry];
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
            s_animation    = anims[i].animation;
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
        s_renderer.render(s_animation, currentTimeTicks(), s_meshRenderer);

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
    if (ImGui::Button(s_playing ? "  ||  " : "  >  ")) s_playing = !s_playing;
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
    ImSequencer::Sequencer(
        &s_seq,
        &s_currentFrame,
        &s_expanded,
        &s_selEntry,
        &s_firstFrame,
        ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_CHANGE_FRAME
    );
    if (s_currentFrame != frameBefore)
        s_elapsedTime = (float)s_currentFrame;
}

// ---- Public API ---------------------------------------------------------

void AnimationViewerWindow::open(Entity entity, AnimationAssetRef animation)
{
    s_entity       = entity;
    s_animation    = animation;
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
    const float seqH     = s_expanded ? 130.f : 40.f;
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
