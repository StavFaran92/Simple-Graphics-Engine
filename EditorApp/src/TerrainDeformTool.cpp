#include "TerrainDeformTool.h"
#include "TerrainBrush.h"
#include "commands/CommandStack.h"

#include "sge.h"

extern Entity g_editorCamera;

Terrain* TerrainDeformTool::getTerrain()
{
	return m_entity.valid() ? m_entity.tryGetComponent<Terrain>() : nullptr;
}

void TerrainDeformTool::onActivate()
{
	Terrain* terrain = getTerrain();
	if (!terrain)
		return;

	TextureResourceRef heightmap = terrain->getHeightmap();
	m_texturePainter.setTexture(heightmap);
	m_texturePainter.setValueRange(Terrain::MIN_HEIGHT, Terrain::MAX_HEIGHT);
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

	Terrain* terrain = getTerrain();
	if (!terrain)
		return;

	m_currentResult = TerrainBrush::raycast(m_entity, *terrain, Ray(rayOrigin, rayDir));
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

bool TerrainDeformTool::onEvent(const Event& e)
{
	if (e.type() == EventType::MouseButtonReleased
		&& static_cast<const MouseButtonReleasedEvent&>(e).button == MouseButton::MOUSE_BUTTON_LEFT)
	{
		endStroke();
		return false;
	}

	Terrain* terrain = getTerrain();
	if (!terrain)
		return false;

	if (Engine::get()->getInput()->getMouse()->getButtonPressed(MouseButton::MOUSE_BUTTON_LEFT))
	{
		TextureResourceRef heightmap = terrain->getHeightmap();

		// First dab: keep the heights from before the stroke
		if (!m_stroking)
		{
			m_strokeStart = takeHeightmapSnapshot(m_entity, { 0, 0, heightmap->getWidth(), heightmap->getHeight() });
			m_strokeRect = {};
			m_stroking = true;
		}

		glm::vec2 offsetPos = TerrainBrush::worldToPixel(m_entity, *terrain, m_currentResult.position, heightmap);

		// Paint on the texture using compute
		m_texturePainter.applyBrush(offsetPos.x, offsetPos.y);
		m_strokeRect = unionRect(m_strokeRect, dabRect(offsetPos, getBrushRadius(), heightmap));

		terrain->syncHeightmap();

		return true;
	}

	return false;
}

void TerrainDeformTool::setMode(Mode mode)
{
	m_mode = mode;
	m_texturePainter.setDrawMode(mode == Mode::Raise ? TexturePainter::DrawMode::Add : TexturePainter::DrawMode::Subtract);
}

void TerrainDeformTool::endStroke()
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
	CommandStack::Instance().record(std::make_unique<EditHeightmapCmd>(std::move(before), std::move(after), "Sculpt terrain"));
}

void TerrainDeformTool::onDeactivate()
{
	endStroke();
}

void TerrainDeformTool::display()
{
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Text("Brush");
	ImGui::Spacing();

	static const char* modeNames[] = { "Raise", "Lower" };
	int mode = (int)getMode();
	if (ImGui::Combo("Mode", &mode, modeNames, IM_ARRAYSIZE(modeNames)))
		setMode((Mode)mode);

	float radius = getBrushRadius();
	if (ImGui::SliderFloat("Radius", &radius, 1.0f, 50.0f))
		setBrushRadius(radius);

	// Height added at the brush centre per dab, times the terrain's scale
	float strength = getBrushStrength();
	if (ImGui::SliderFloat("Strength", &strength, 0.0f, 5.0f))
		setBrushStrength(strength);
}
