#include "TerrainDeformTool.h"

#include "sge.h"

extern Entity g_editorCamera;
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
	ImVec2 mousePos = ImGui::GetMousePos();
	ImVec2 viewportOffset = ImGui::GetWindowContentRegionMin();
	ImVec2 viewportPosOffset{ viewportPos.x + viewportOffset.x, viewportPos.y + viewportOffset.y };

	// We alter the mouse position from small window into full screen (the renderered object pick texture)
	int alteredX = (mousePos.x - viewportPosOffset.x) / viewportSize.x * Engine::get()->getWindow()->getWidth();
	int alteredY = (mousePos.y - viewportPosOffset.y) / viewportSize.y * Engine::get()->getWindow()->getHeight();

	auto& cameraComponent = g_editorCamera.getComponent<CameraComponent>();
	auto& cameraTransform = g_editorCamera.getComponent<Transformation>();
	glm::mat4 view = glm::lookAt(cameraTransform.getWorldPosition(), cameraTransform.getWorldPosition() + cameraComponent.front, cameraComponent.up);
	auto projection = g_editorCamera.getComponent<CameraComponent>().getProjection();

	auto& [rayOrigin, rayDir] = Math3D::ScreenPointToRay(alteredX, alteredY, Engine::get()->getWindow()->getWidth(), Engine::get()->getWindow()->getHeight(), view, projection);

	m_currentResult = g_activeTerrain->raycast(Ray(rayOrigin, rayDir));
	if (m_currentResult.hit)
	{
		Engine::get()->getSubSystem<Graphics>()->view = view;
		Engine::get()->getSubSystem<Graphics>()->projection = projection;
		//logDebug("ray origin {},{},{}, ray dir {},{},{}, hit position {},{},{}", rayOrigin.x, rayOrigin.y, rayOrigin.z, rayDir.x, rayDir.y, rayDir.z, results.position.x, results.position.y, results.position.z);
		unsigned int editorFrameBufferID = Engine::get()->getContext()->getActiveScene()->getRenderViewFrameBufferID("Editor View");
		glBindFramebuffer(GL_FRAMEBUFFER, editorFrameBufferID);
		DebugHelper::getInstance().drawLine(m_currentResult.position, m_currentResult.position + glm::vec3(0, 100, 0), glm::vec3(1, 0, 0), 3.f);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

bool TerrainDeformTool::onEvent(SDL_Event e)
{
	if (Engine::get()->getInput()->getMouse()->getButtonPressed(MouseButton::MOUSE_BUTTON_LEFT))
	{
		glm::vec2 offsetPos = glm::vec2(m_currentResult.position.x, m_currentResult.position.z);
		offsetPos.x += g_activeTerrain->getWidth() * .5f;
		offsetPos.y += g_activeTerrain->getHeight() * .5f;
		m_texturePainter.applyBrush(offsetPos.x, offsetPos.y);

		return true;
	}

	return false;
}
