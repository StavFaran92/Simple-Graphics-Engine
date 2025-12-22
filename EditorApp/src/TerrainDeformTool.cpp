#include "TerrainDeformTool.h"

#include "sge.h"

extern Terrain* g_activeTerrain;

void TerrainDeformTool::onActivate()
{
	if (!g_activeTerrain)
		return;

	ResourceWrapper<Texture> heightmap = g_activeTerrain->getHeightmap();
	m_texturePainter.setTexture(heightmap);
}

void TerrainDeformTool::update(ImVec2 viewportPos, ImVec2 viewportSize)
{
}

bool TerrainDeformTool::onEvent(SDL_Event e)
{
	if (Engine::get()->getInput()->getMouse()->getButtonPressed(MouseButton::MOUSE_BUTTON_LEFT))
	{
		//glm::vec2 offsetPos = glm::vec2(m_currentResult.position.x, m_currentResult.position.z);
		//offsetPos.x += g_activeTerrain->getWidth() * .5f;
		//offsetPos.y += g_activeTerrain->getHeight() * .5f;
		static int y = 0;
		y++;
		m_texturePainter.applyBrush(y, y);

		return true;
	}

	return false;
}
