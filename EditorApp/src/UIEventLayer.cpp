#include "UIEventLayer.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

bool UIEventLayer::handleEvent(SDL_Event e)
{
    if (!m_isEnabled)
        return false;

    ImGui_ImplSDL2_ProcessEvent(&e);

    auto iter = m_listeners.find((SDL_EventType)e.type);
    if (iter != m_listeners.end())
    {
        for (auto& ec : iter->second)
        {
            ec.func(e);
        }
    }

    auto& io = ImGui::GetIO();
    return (io.WantCaptureMouse || io.WantCaptureKeyboard);
}