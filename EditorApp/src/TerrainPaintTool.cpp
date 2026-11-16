#include "TerrainPaintTool.h"
#include "TerrainBrush.h"

#include "sge.h"

Terrain* TerrainPaintTool::getTerrain()
{
    return m_entity.valid() ? m_entity.tryGetComponent<Terrain>() : nullptr;
}

void TerrainPaintTool::setEntity(Entity entity)
{
    endStroke();
    m_entity = entity;
}

void TerrainPaintTool::onActivate()
{
    m_selectedLayerIndex = 1;
    if (Terrain* terrain = getTerrain())
        clampSelection(terrain->getLayerCount());
}

bool TerrainPaintTool::dabAt(const glm::vec3& worldPoint)
{
    Terrain* terrain = getTerrain();
    if (!terrain)
        return false;

    // The base layer has no mask to paint
    if (m_selectedLayerIndex < 1 || m_selectedLayerIndex >= (int)terrain->getLayerCount())
        return false;

    glm::vec2 terrainPos = TerrainBrush::worldToTerrain(m_entity, *terrain, worldPoint);
    if (terrainPos.x < 0.f || terrainPos.y < 0.f || terrainPos.x > terrain->getWidth() || terrainPos.y > terrain->getHeight())
        return false;

    // Bound on every dab, so a removed layer or resize never leaves a stale mask
    LayerMask layer = terrain->getLayer(m_selectedLayerIndex);
    TextureAssetRef& mask = layer.mask;
    TextureResourceRef maskTexture = mask.resource();
    if (maskTexture.isEmpty())
        return false;
    m_texturePainter.setTexture(maskTexture);

    // Another layer selected, or the layers shifted: the open stroke belongs to another mask
    if (m_stroke.isOpen() && m_stroke.texture() != mask.getUID())
        endStroke();
    m_stroke.begin(mask);

    glm::vec2 pixel = TerrainBrush::worldToPixel(m_entity, *terrain, worldPoint, maskTexture);
    m_texturePainter.applyBrush(pixel.x, pixel.y);
    m_stroke.addDab(pixel, getBrushRadius());

    maskTexture->download();
    mask.makeDirty();
    return true;
}

void TerrainPaintTool::endStroke(const std::string& label)
{
    m_stroke.end(label);
}

bool TerrainPaintTool::selectLayer(int index)
{
    Terrain* terrain = getTerrain();
    if (!terrain)
        return false;

    if (index >= (int)terrain->getLayerCount() || index < 0)
    {
        logWarning("Invalid layer specified: {}", index);
        return false;
    }

    // cannot alter base layer
    if (index == 0)
        return false;

    m_selectedLayerIndex = index;
    return true;
}

void TerrainPaintTool::setDrawMode(DrawMode mode)
{
    m_drawMode = mode;

    if (mode == DrawMode::Draw)
    {
        m_texturePainter.setDrawMode(TexturePainter::DrawMode::Add);
    }
    else if (mode == DrawMode::Erase)
    {
        m_texturePainter.setDrawMode(TexturePainter::DrawMode::Subtract);
    }
}
