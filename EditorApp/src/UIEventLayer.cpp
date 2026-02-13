#include "UIEventLayer.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "EditorState.h"

bool shouldSceneViewGetKeyboard()
{
    const ImGuiIO& io = ImGui::GetIO();
    const auto& state = EditorState::Instance();

    bool uiFocused =
        //ImGui::IsAnyItemActive() ||  // UI item is being edited
        //ImGui::IsAnyItemFocused() ||  // UI has keyboard focus
        io.WantTextInput;              // text widgets

    // If UI needs keyboard -> block scene
    if (uiFocused)
        return false;

    // If mouse is NOT in scene view -> block scene
    if (!state.isMouseInSceneView)
        return false;

    // Otherwise: Override ImGui's WantCaptureKeyboard
    return true;
}

bool UIEventLayer::handleEvent(SDL_Event e)
{
    if (!m_isEnabled)
        return false;

    //if (e.type == SDL_MOUSEBUTTONDOWN)
    //    std::cout << "\n";

    ImGui_ImplSDL2_ProcessEvent(&e);

    bool isHandled = false;

    //EditorState::Instance().uiCapture

    if (!shouldSceneViewGetKeyboard())
    {
        isHandled = true; // block event
    }

    //auto iter = m_listeners.find((SDL_EventType)e.type);
    //if (iter != m_listeners.end())
    //{
    //    for (auto& ec : iter->second)
    //    {
    //        ec.func(e);
    //    }
    //}

    return isHandled;
}