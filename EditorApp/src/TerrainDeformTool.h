#pragma once

#include "imgui.h"

#include "EditorTool.h"
#include "core/Subscriber.h"

#include "component/Terrain.h"
#include "sge.h"
#include "commands/TerrainCommands.h"

class TerrainDeformTool : public EditorTool
{
public:
    TerrainDeformTool() = default;

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

    enum class Mode { Raise, Lower };

    void setBrushRadius(float r) { m_texturePainter.setBrushRadius(r); }
    float getBrushRadius() const { return m_texturePainter.getBrushRadius(); }

    void setBrushStrength(float s) { m_texturePainter.setBrushStrength(s); }
    float getBrushStrength() const { return m_texturePainter.getBrushStrength(); }

    void setMode(Mode mode);
    Mode getMode() const { return m_mode; }

private:
    // The entity's terrain, or null when it's gone
    Terrain* getTerrain();

    // Records the open stroke as one undo step
    void endStroke();

    RayHit m_currentResult;
    TexturePainter m_texturePainter;
    Mode m_mode = Mode::Raise;

    bool m_stroking = false;
    HeightmapSnapshot m_strokeStart;  // whole heightmap from before the stroke
    HeightmapRect m_strokeRect;       // pixels the stroke touched so far
};