#pragma once

#include "imgui.h"

class EditorTool 
{
public:
    enum class Type
    {
        None,
        TransformTool,
        FoliagePainter
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
};