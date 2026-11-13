#pragma once

#include "EditorTool.h"
#include "TerrainDeformTool.h"

// The editor's Deformer: mouse and Inspector on top of TerrainDeformTool
class TerrainDeformToolUI : public EditorTool
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
        return "TerrainDeformerTool";
    }

private:
    TerrainDeformTool m_tool;
    RayHit m_currentResult;
};
