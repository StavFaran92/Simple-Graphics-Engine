#include "TerrainDeformToolUI.h"
#include "TerrainBrush.h"

#include "sge.h"

#include "imgui.h"

extern Entity g_editorCamera;

void TerrainDeformToolUI::onActivate()
{
	m_tool.setEntity(m_entity);
}

void TerrainDeformToolUI::onDeactivate()
{
	m_tool.endStroke();
}

void TerrainDeformToolUI::update(ImVec2 viewportPos, ImVec2 viewportSize)
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

	Terrain* terrain = m_entity.valid() ? m_entity.tryGetComponent<Terrain>() : nullptr;
	if (!terrain)
		return;

	m_currentResult = TerrainBrush::raycast(m_entity, *terrain, Ray(rayOrigin, rayDir));
	if (m_currentResult.hit)
	{
		Engine::get()->getSubSystem<Graphics>()->view = view;
		Engine::get()->getSubSystem<Graphics>()->projection = projection;
		unsigned int editorFrameBufferID = Engine::get()->getContext()->getActiveScene()->getRenderViewFrameBufferID("Editor View");
		glBindFramebuffer(GL_FRAMEBUFFER, editorFrameBufferID);
		DebugHelper::getInstance().drawLine(m_currentResult.position, m_currentResult.position + glm::vec3(0, 100, 0), glm::vec3(1, 0, 0), 3.f);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

bool TerrainDeformToolUI::onEvent(const Event& e)
{
	if (e.type() == EventType::MouseButtonReleased
		&& static_cast<const MouseButtonReleasedEvent&>(e).button == MouseButton::MOUSE_BUTTON_LEFT)
	{
		m_tool.endStroke();
		return false;
	}

	// Holding the button paints under the cursor; the first dab starts the stroke
	if (Engine::get()->getInput()->getMouse()->getButtonPressed(MouseButton::MOUSE_BUTTON_LEFT) && m_currentResult.hit)
		return m_tool.dabAt(m_currentResult.position);

	return false;
}

void TerrainDeformToolUI::display()
{
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Text("Brush");
	ImGui::Spacing();

	static const char* modeNames[] = { "Raise", "Lower" };
	int mode = (int)m_tool.getMode();
	if (ImGui::Combo("Mode", &mode, modeNames, IM_ARRAYSIZE(modeNames)))
		m_tool.setMode((TerrainDeformTool::Mode)mode);

	float radius = m_tool.getBrushRadius();
	if (ImGui::SliderFloat("Radius", &radius, 1.0f, 50.0f))
		m_tool.setBrushRadius(radius);

	// Height added at the brush centre per dab, times the terrain's scale
	float strength = m_tool.getBrushStrength();
	if (ImGui::SliderFloat("Strength", &strength, 0.0f, 5.0f))
		m_tool.setBrushStrength(strength);
}
