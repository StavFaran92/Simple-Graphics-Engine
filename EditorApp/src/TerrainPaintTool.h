#pragma once

#include "imgui.h"

#include "EditorTool.h"
#include "core/Subscriber.h"

#include "component/Terrain.h"
#include "sge.h"

class TerrainPaintTool : public EditorTool
{
public:
    TerrainPaintTool() = default;

    void onActivate() override;

    void update(ImVec2 viewportPos, ImVec2 viewportSize) override;

    bool onEvent(SDL_Event e);

    const char* name() const override
    {
        return "TerrainPainterTool";
    }

private:
    RayHit m_currentResult;
};
