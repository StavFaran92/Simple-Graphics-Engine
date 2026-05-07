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

    void selectLayer(int index) { m_selectedLayerIndex = index; }
    int getSelectedLayer() const { return m_selectedLayerIndex; }
    void clampSelection(size_t layerCount) {
        if (m_selectedLayerIndex >= (int)layerCount)
            m_selectedLayerIndex = (int)layerCount - 1;
        if (m_selectedLayerIndex < 0)
            m_selectedLayerIndex = 0;
    }

private:
    int m_selectedLayerIndex = 0;
    RayHit m_currentResult;
    TexturePainter m_texturePainter;
};
