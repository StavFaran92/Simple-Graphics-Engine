#pragma once

#include "imgui.h"

#include "EditorTool.h"
#include "core/Subscriber.h"

#include "component/Terrain.h"
#include "sge.h"

class TerrainDeformTool : public EditorTool
{
public:
    TerrainDeformTool() = default;

    void onActivate() override;

    void update(ImVec2 viewportPos, ImVec2 viewportSize) override;

    bool onEvent(const Event& e) override;

    const char* name() const override
    {
        return "TerrainDeformerTool";
    }

private:
    RayHit m_currentResult;
    TexturePainter m_texturePainter;
};