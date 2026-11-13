#pragma once

#include "component/Terrain.h"
#include "sge.h"
#include "commands/TerrainCommands.h"

#include <string>

// Raises or lowers a terrain's heightmap; no UI, so the editor and MCP both drive it
class TerrainDeformTool
{
public:
    TerrainDeformTool();

    enum class Mode { Raise, Lower };

    // The terrain to sculpt; closes an open stroke first
    void setEntity(Entity entity);

    // In heightmap pixels
    void setBrushRadius(float r) { m_texturePainter.setBrushRadius(r); }
    float getBrushRadius() const { return m_texturePainter.getBrushRadius(); }

    // Height added at the brush centre per dab, before the terrain's scale
    void setBrushStrength(float s) { m_texturePainter.setBrushStrength(s); }
    float getBrushStrength() const { return m_texturePainter.getBrushStrength(); }

    void setMode(Mode mode);
    Mode getMode() const { return m_mode; }

    // A stroke is one undo step: begin, any number of dabs, end
    void beginStroke();

    // One dab at a world point, starting a stroke if none is open; false when the point is off the terrain
    bool dabAt(const glm::vec3& worldPoint);

    // Records the open stroke as one undo step
    void endStroke(const std::string& label = "Sculpt terrain");

private:
    // The entity's terrain, or null when it's gone
    Terrain* getTerrain();

    Entity m_entity;
    TexturePainter m_texturePainter;
    Mode m_mode = Mode::Raise;

    bool m_stroking = false;
    HeightmapSnapshot m_strokeStart;  // whole heightmap from before the stroke
    HeightmapRect m_strokeRect;       // pixels the stroke touched so far
};