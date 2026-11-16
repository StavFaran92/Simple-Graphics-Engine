#pragma once

#include "component/Terrain.h"
#include "sge.h"
#include "commands/TerrainCommands.h"

// Paints a terrain's layer masks; no UI, so the editor and MCP both drive it
class TerrainPaintTool
{
public:
    // The terrain to paint; closes an open stroke first
    void setEntity(Entity entity);
    void onActivate();

    // False for the base layer 0 or a layer that doesn't exist
    bool selectLayer(int index);
    int getSelectedLayer() const { return m_selectedLayerIndex; }
    void clampSelection(size_t layerCount) {
        if (m_selectedLayerIndex >= (int)layerCount)
            m_selectedLayerIndex = (int)layerCount - 1;
        if (m_selectedLayerIndex < 1)
            m_selectedLayerIndex = 1;
    }

    enum class DrawMode { Draw, Erase };

    void setBrushRadius(float r) { m_texturePainter.setBrushRadius(r); }
    float getBrushRadius() const { return m_texturePainter.getBrushRadius(); }

    void setBrushStrength(float s) { m_texturePainter.setBrushStrength(s); }
    float getBrushStrength() const { return m_texturePainter.getBrushStrength(); }

    void setDrawMode(DrawMode mode);
    DrawMode getDrawMode() const { return m_drawMode; }

    // One dab on the selected layer at a world point; false when off the terrain or no paintable layer
    bool dabAt(const glm::vec3& worldPoint);

    // Records the open stroke as one undo step; dabAt opens one when needed
    void endStroke(const std::string& label = "Paint terrain");

private:
    // The entity's terrain, or null when it's gone
    Terrain* getTerrain();

    Entity m_entity;
    int m_selectedLayerIndex = 0;
    TexturePainter m_texturePainter;
    DrawMode m_drawMode = DrawMode::Draw;
    TextureStroke m_stroke;
};
