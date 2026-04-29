#include "TerrainPaintTool.h"

#include "sge.h"

extern Entity g_editorCamera;
extern Terrain* g_activeTerrain;

void TerrainPaintTool::onActivate()
{
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
    return false;
}
