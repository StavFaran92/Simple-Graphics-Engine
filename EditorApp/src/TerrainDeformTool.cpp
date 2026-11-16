#include "TerrainDeformTool.h"
#include "TerrainBrush.h"

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
	Terrain* terrain = getTerrain();
	if (!terrain)
		return;

	m_stroke.begin(terrain->getHeightmapAsset());
}

bool TerrainDeformTool::dabAt(const glm::vec3& worldPoint)
{
	Terrain* terrain = getTerrain();
	if (!terrain)
		return false;

	glm::vec2 terrainPos = TerrainBrush::worldToTerrain(m_entity, *terrain, worldPoint);
	if (terrainPos.x < 0.f || terrainPos.y < 0.f || terrainPos.x > terrain->getWidth() || terrainPos.y > terrain->getHeight())
		return false;

	// A resize mid-drag makes a new heightmap; what follows is a stroke of its own
	if (m_stroke.isOpen() && m_stroke.texture() != terrain->getHeightmapAsset().getUID())
		endStroke();

	beginStroke();
	if (!m_stroke.isOpen())
		return false;

	// Bound on every dab, so a resized terrain's new heightmap is the one painted
	TextureResourceRef heightmap = terrain->getHeightmap();
	m_texturePainter.setTexture(heightmap);

	glm::vec2 pixel = TerrainBrush::worldToPixel(m_entity, *terrain, worldPoint, heightmap);
	m_texturePainter.applyBrush(pixel.x, pixel.y);
	m_stroke.addDab(pixel, getBrushRadius());

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
	m_stroke.end(label);
}
