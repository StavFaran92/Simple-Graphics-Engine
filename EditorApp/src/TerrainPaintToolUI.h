#pragma once

#include "EditorTool.h"
#include "TerrainPaintTool.h"

// The editor's Painter: mouse and Inspector on top of TerrainPaintTool
class TerrainPaintToolUI : public EditorTool
{
public:
    void onActivate() override;

    // Records a stroke that's still open
    void onDeactivate() override;

    void update(ImVec2 viewportPos, ImVec2 viewportSize) override;

    bool onEvent(const Event& e) override;

    void display() override;

    const char* name() const override
    {
        return "TerrainPainterTool";
    }

private:
    TerrainPaintTool m_tool;
    RayHit m_currentResult;
};
