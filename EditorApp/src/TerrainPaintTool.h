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

    bool onEvent(const Event& e) override;

    const char* name() const override
    {
        return "TerrainPainterTool";
    }

    void selectLayer(int index);
    int getSelectedLayer() const { return m_selectedLayerIndex; }
    void clampSelection(size_t layerCount) {
        if (m_selectedLayerIndex >= (int)layerCount)
            m_selectedLayerIndex = (int)layerCount - 1;
        if (m_selectedLayerIndex < 0)
            m_selectedLayerIndex = 0;
    }

    enum class DrawMode { Draw, Erase };

    void setBrushRadius(float r) { m_texturePainter.setBrushRadius(r); }
    float getBrushRadius() const { return m_texturePainter.getBrushRadius(); }

    void setBrushStrength(float s) { m_texturePainter.setBrushStrength(s); }
    float getBrushStrength() const { return m_texturePainter.getBrushStrength(); }

    void setDrawMode(DrawMode mode);
    DrawMode getDrawMode() const { return m_drawMode; }

private:
    int m_selectedLayerIndex = 0;
    RayHit m_currentResult;
    TexturePainter m_texturePainter;
    DrawMode m_drawMode = DrawMode::Draw;
};
