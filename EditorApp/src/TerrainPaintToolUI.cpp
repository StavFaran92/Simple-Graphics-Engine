#include "TerrainPaintToolUI.h"
#include "TerrainBrush.h"
#include "Thumbnails/ThumbnailCache.h"

#include "sge.h"

#include "imgui.h"

extern Entity g_editorCamera;

void TerrainPaintToolUI::onActivate()
{
	m_tool.setEntity(m_entity);
	m_tool.onActivate();
}

void TerrainPaintToolUI::onDeactivate()
{
	m_tool.endStroke();
}

void TerrainPaintToolUI::update(ImVec2 viewportPos, ImVec2 viewportSize)
{
	ImVec2 mousePos = ImGui::GetMousePos();
	ImVec2 viewportOffset = ImGui::GetWindowContentRegionMin();
	ImVec2 viewportPosOffset{ viewportPos.x + viewportOffset.x, viewportPos.y + viewportOffset.y };

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
		DebugHelper::getInstance().drawLine(m_currentResult.position, m_currentResult.position + glm::vec3(0, 100, 0), glm::vec3(0, 0, 1), 3.f);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

bool TerrainPaintToolUI::onEvent(const Event& e)
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

void TerrainPaintToolUI::display()
{
	Terrain* terrain = m_entity.valid() ? m_entity.tryGetComponent<Terrain>() : nullptr;
	if (!terrain)
		return;

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Text("Layers");
	ImGui::Spacing();

	int layerCount = terrain->getLayerCount();

	// Clamp selection in case layers changed
	m_tool.clampSelection(layerCount);

	for (int i = 0; i < layerCount; i++)
	{
		ImGui::PushID(i);

		bool checked = (i == m_tool.getSelectedLayer());

		// The base layer can't be painted
		ImGui::BeginDisabled(i == 0);
		if (ImGui::Checkbox("##layer", &checked) && checked)
			m_tool.selectLayer(i);
		ImGui::EndDisabled();

		ImTextureID thumbnail = 0;
		if (!terrain->m_material.isEmpty())
		{
			auto layers = terrain->m_material->getAllProptiesOfType(MaterialPropertyType::TERRAIN_LAYER);
			if (auto it = layers.find("terrainLayers[" + std::to_string(i) + "]"); it != layers.end())
			{
				auto terrainLayer = std::get<std::shared_ptr<TerrainLayerAsset>>(it->second);
				if (terrainLayer && !terrainLayer->albedoTexture.isEmpty())
					thumbnail = ThumbnailCache::Instance().get(terrainLayer->albedoTexture.getUID());
			}
		}
		if (!thumbnail)
			thumbnail = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_BLACK).resource()->getID()));
		ImGui::SameLine();
		ImGui::Image(thumbnail, ImVec2(20, 20));
		ImGui::SameLine();
		ImGui::Text("Layer %d", i);

		ImGui::PopID();
	}

	ImGui::Spacing();

	// An open stroke is recorded before the layer list changes, so history keeps the order
	if (ImGui::Button("+"))
	{
		m_tool.endStroke();
		terrain->addLayer();
		m_tool.selectLayer(terrain->getLayerCount() - 1);
	}

	ImGui::SameLine();

	// Disabled when only the base layer remains
	ImGui::BeginDisabled(layerCount <= 1);
	if (ImGui::Button("-"))
	{
		m_tool.endStroke();
		terrain->removeLayer(m_tool.getSelectedLayer());
		m_tool.clampSelection(terrain->getLayerCount());
	}
	ImGui::EndDisabled();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Text("Brush");
	ImGui::Spacing();

	static const char* drawModeNames[] = { "Draw", "Erase" };
	int drawMode = (int)m_tool.getDrawMode();
	if (ImGui::Combo("Mode", &drawMode, drawModeNames, IM_ARRAYSIZE(drawModeNames)))
		m_tool.setDrawMode((TerrainPaintTool::DrawMode)drawMode);

	float radius = m_tool.getBrushRadius();
	if (ImGui::SliderFloat("Radius", &radius, 1.0f, 50.0f))
		m_tool.setBrushRadius(radius);

	float strength = m_tool.getBrushStrength();
	if (ImGui::SliderFloat("Strength", &strength, 0.0f, 1.0f))
		m_tool.setBrushStrength(strength);
}
