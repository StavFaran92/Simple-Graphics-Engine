#include "UIEventLayer.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "EditorState.h"

bool UIEventLayer::handleEvent(SDL_Event e)
{
    if (!m_isEnabled)
        return false;

    ImGui_ImplSDL2_ProcessEvent(&e);

    ImVec2 mouse = ImGui::GetMousePos();

    bool isHandled = false;

    // If mouse is inside scene view -> DO NOT BLOCK
    bool shouldCaptureMouse = !EditorState::Instance().sceneViewRect.contains(glm::vec2(mouse.x, mouse.y));

    //auto iter = m_listeners.find((SDL_EventType)e.type);
    //if (iter != m_listeners.end())
    //{
    //    for (auto& ec : iter->second)
    //    {
    //        ec.func(e);
    //    }
    //}

    auto& io = ImGui::GetIO();
    return (shouldCaptureMouse || io.WantCaptureKeyboard);
}