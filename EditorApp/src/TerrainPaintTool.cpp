#include "TerrainPaintTool.h"
#include "TerrainBrush.h"

#include "sge.h"

extern Entity g_editorCamera;

Terrain* TerrainPaintTool::getTerrain()
{
    return m_entity.valid() ? m_entity.tryGetComponent<Terrain>() : nullptr;
}

void TerrainPaintTool::onActivate()
{
    Terrain* terrain = getTerrain();
    if (!terrain)
        return;

    auto layerMask = terrain->getLayer(m_selectedLayerIndex);
    m_texturePainter.setTexture(layerMask.mask.resource());
    //m_texturePainter.setBrushRadius(2.f);


}

void TerrainPaintTool::update(ImVec2 viewportPos, ImVec2 viewportSize)
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

    Terrain* terrain = getTerrain();
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

bool TerrainPaintTool::onEvent(const Event& e)
{
    Terrain* terrain = getTerrain();
    if (!terrain)
        return false;

    if (Engine::get()->getInput()->getMouse()->getButtonPressed(MouseButton::MOUSE_BUTTON_LEFT))
    {
        TextureResourceRef mask = m_texturePainter.getBoundTexture();
        if (mask.isEmpty())
            return false;

        glm::vec2 offsetPos = TerrainBrush::worldToPixel(m_entity, *terrain, m_currentResult.position, mask);

        // Paint on the texture using compute
        m_texturePainter.applyBrush(offsetPos.x, offsetPos.y);

        // TODO should move to on mouse released
        terrain->getLayer(m_selectedLayerIndex).mask.resource()->download();
        terrain->getLayer(m_selectedLayerIndex).mask.makeDirty();

        //auto res = g_activeTerrain->m_material->getProperty<std::shared_ptr<TerrainLayerAsset>>("terrainLayers[1]")->mask.resource();
        //res->download();
        //g_activeTerrain->m_material.makeDirty();



        return true;
    }

    return false;
}

void TerrainPaintTool::selectLayer(int index)
{
    Terrain* terrain = getTerrain();
    if (!terrain)
        return;

    if (index > terrain->getLayerCount() || index < 0)
    {
        logWarning("Invalid layer specified: {}", index);
        return;
    }

    // cannot alter base layer
    if (index == 0)
        return;

    m_selectedLayerIndex = index;

    auto layerMask = terrain->getLayer(m_selectedLayerIndex);
    m_texturePainter.setTexture(layerMask.mask.resource());
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

void TerrainPaintTool::display()
{
    Terrain* terrain = getTerrain();
    if (!terrain)
        return;

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Layers");
    ImGui::Spacing();

    int layerCount = terrain->getLayerCount();

    // Clamp selection in case layers changed
    clampSelection(layerCount);

    for (int i = 0; i < layerCount; i++)
    {
        ImGui::PushID(i);

        bool isSelected = (i == getSelectedLayer());
        ImGui::Bullet();
        ImGui::SameLine();

        const LayerMask& layer = terrain->getLayer(i);
        std::string label = layer.name;

        if (ImGui::Selectable(label.c_str(), isSelected))
            selectLayer(i);

        ImGui::PopID();
    }

    ImGui::Spacing();

    // Add button
    if (ImGui::Button("+"))
    {
        terrain->addLayer();
    }

    ImGui::SameLine();

    // Remove button — disabled when only 1 layer remains
    ImGui::BeginDisabled(layerCount <= 1);
    if (ImGui::Button("-"))
    {
        terrain->removeLayer(getSelectedLayer());
        clampSelection(terrain->getLayerCount());
    }
    ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Brush");
    ImGui::Spacing();

    static const char* drawModeNames[] = { "Draw", "Erase" };
    int drawMode = (int)getDrawMode();
    if (ImGui::Combo("Mode", &drawMode, drawModeNames, IM_ARRAYSIZE(drawModeNames)))
        setDrawMode((DrawMode)drawMode);

    float radius = getBrushRadius();
    if (ImGui::SliderFloat("Radius", &radius, 1.0f, 50.0f))
        setBrushRadius(radius);

    float strength = getBrushStrength();
    if (ImGui::SliderFloat("Strength", &strength, 0.0f, 1.0f))
        setBrushStrength(strength);
}
