#include "TerrainDeformTool.h"
#include "TerrainBrush.h"
#include "commands/CommandStack.h"

#include "sge.h"

TerrainDeformTool::TerrainDeformTool()
{
	m_texturePainter.setValueRange(Terrain::MIN_HEIGHT, Terrain::MAX_HEIGHT);
}

Terrain* TerrainDeformTool::getTerrain()
{
	return m_entity.valid() ? m_entity.tryGetComponent<Terrain>() : nullptr;
}

void TerrainDeformTool::setEntity(Entity entity)
{
	endStroke();
	m_entity = entity;
}

void TerrainDeformTool::beginStroke()
{
	if (m_stroking)
		return;

	Terrain* terrain = getTerrain();
	if (!terrain)
		return;

	TextureResourceRef heightmap = terrain->getHeightmap();
	if (heightmap.isEmpty())
		return;

	// Keep the heights from before the stroke
	m_strokeStart = takeHeightmapSnapshot(m_entity, { 0, 0, heightmap->getWidth(), heightmap->getHeight() });
	m_strokeRect = {};
	m_stroking = true;
}

bool TerrainDeformTool::dabAt(const glm::vec3& worldPoint)
{
	Terrain* terrain = getTerrain();
	if (!terrain)
		return false;

	glm::vec2 terrainPos = TerrainBrush::worldToTerrain(m_entity, *terrain, worldPoint);
	if (terrainPos.x < 0.f || terrainPos.y < 0.f || terrainPos.x > terrain->getWidth() || terrainPos.y > terrain->getHeight())
		return false;

	beginStroke();
	if (!m_stroking)
		return false;

	// Bound on every dab, so a resized terrain's new heightmap is the one painted
	TextureResourceRef heightmap = terrain->getHeightmap();
	m_texturePainter.setTexture(heightmap);

	glm::vec2 pixel = TerrainBrush::worldToPixel(m_entity, *terrain, worldPoint, heightmap);
	m_texturePainter.applyBrush(pixel.x, pixel.y);
	m_strokeRect = unionRect(m_strokeRect, dabRect(pixel, getBrushRadius(), heightmap));

	terrain->syncHeightmap();
	return true;
}

void TerrainDeformTool::setMode(Mode mode)
{
	m_mode = mode;
	m_texturePainter.setDrawMode(mode == Mode::Raise ? TexturePainter::DrawMode::Add : TexturePainter::DrawMode::Subtract);
}

void TerrainDeformTool::endStroke(const std::string& label)
{
	if (!m_stroking)
		return;

	m_stroking = false;
	HeightmapSnapshot start = std::move(m_strokeStart);
	m_strokeStart = {};

	// The copy only fits the heightmap it was taken from
	Terrain* terrain = getTerrain();
	if (!terrain || terrain->getHeightmapAsset().getUID() != start.heightmap)
		return;

	if (m_strokeRect.width <= 0 || m_strokeRect.height <= 0)
		return;

	HeightmapSnapshot before = cropSnapshot(start, m_strokeRect);
	HeightmapSnapshot after = takeHeightmapSnapshot(m_entity, m_strokeRect);
	CommandStack::Instance().record(std::make_unique<EditHeightmapCmd>(std::move(before), std::move(after), label));
}
