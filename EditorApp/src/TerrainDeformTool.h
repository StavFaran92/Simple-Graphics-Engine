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

    enum class Mode { Raise, Lower };

    void setBrushRadius(float r) { m_texturePainter.setBrushRadius(r); }
    float getBrushRadius() const { return m_texturePainter.getBrushRadius(); }

    void setBrushStrength(float s) { m_texturePainter.setBrushStrength(s); }
    float getBrushStrength() const { return m_texturePainter.getBrushStrength(); }

    void setMode(Mode mode);
    Mode getMode() const { return m_mode; }

private:
    RayHit m_currentResult;
    TexturePainter m_texturePainter;
    Mode m_mode = Mode::Raise;
};