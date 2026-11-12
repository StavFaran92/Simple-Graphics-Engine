#pragma once

#include "imgui.h"

#include "core/Subscriber.h"
#include "runtime/Entity.h"

class EditorTool : public Subscriber
{
public:
    enum class Type
    {
        None,
        TransformTool,
        FoliagePainter,
        TerrainDeformer,
        TerrainPainter,
    };
public:
    virtual ~EditorTool() = default;

    virtual void onActivate() {}
    virtual void onDeactivate() {}

    virtual void update(ImVec2 viewportPos, ImVec2 viewportSize) = 0;


    // Maybe in the future..
    //virtual void update(const EditorContext& ctx) = 0;
    //virtual void onInput(const EditorInput& input) = 0;

    virtual const char* name() const = 0;

    // The entity the tool works on; each tool takes what it needs from it
    void setEntity(Entity entity) { m_entity = entity; }

    // The tool's own settings, drawn in the Inspector
    virtual void display() {}

protected:
    Entity m_entity;
};