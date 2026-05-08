#include "TerrainPaintTool.h"

#include "sge.h"

extern Entity g_editorCamera;
extern Terrain* g_activeTerrain;

void TerrainPaintTool::onActivate()
{
    if (!g_activeTerrain)
        return;

    //if (g_activeTerrain->getLayers().size() < 2)
    //    return;

    auto layerMask = g_activeTerrain->getLayer(m_selectedLayerIndex);
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

    if (!g_activeTerrain)
        return;

    m_currentResult = g_activeTerrain->raycast(Ray(rayOrigin, rayDir));
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

bool TerrainPaintTool::onEvent(SDL_Event e)
{
    if (Engine::get()->getInput()->getMouse()->getButtonPressed(MouseButton::MOUSE_BUTTON_LEFT))
    {
        glm::vec2 offsetPos = glm::vec2(m_currentResult.position.x, m_currentResult.position.z);
        offsetPos.x += g_activeTerrain->getWidth() * .5f;
        offsetPos.y += g_activeTerrain->getHeight() * .5f;

        // Paint on the texture using compute
        m_texturePainter.applyBrush(offsetPos.x, offsetPos.y);

        // TODO should move to on mouse released
        g_activeTerrain->getLayer(m_selectedLayerIndex).mask.resource()->download();
        g_activeTerrain->getLayer(m_selectedLayerIndex).mask.makeDirty();

        //auto res = g_activeTerrain->m_material->getProperty<std::shared_ptr<TerrainLayerAsset>>("terrainLayers[1]")->mask.resource();
        //res->download();
        //g_activeTerrain->m_material.makeDirty();



        return true;
    }

    return false;
}

void TerrainPaintTool::selectLayer(int index)
{
    if (index > g_activeTerrain->getLayerCount() || index < 0)
    {
        logWarning("Invalid layer specified: {}", index);
        return;
    }

    m_selectedLayerIndex = index;

    auto layerMask = g_activeTerrain->getLayer(m_selectedLayerIndex);
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
