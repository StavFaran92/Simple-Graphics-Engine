#include "TerrainDeformerTool.h"

#include "sge.h"

void TerrainDeformerTool::update(ImVec2 viewportPos, ImVec2 viewportSize)
{
}

bool TerrainDeformerTool::onEvent(SDL_Event e)
{
	if (Engine::get()->getInput()->getMouse()->getButtonPressed(MouseButton::MOUSE_BUTTON_LEFT))
	{
		//glm::vec2 offsetPos = glm::vec2(m_currentResult.position.x, m_currentResult.position.z);
		//offsetPos.x += g_activeTerrain->getWidth() * .5f;
		//offsetPos.y += g_activeTerrain->getHeight() * .5f;
		m_texturePainter.applyBrush(0, 0);

		return true;
	}

	return false;
}
