#include "UIEventLayer.h"

#include "imgui.h"
#include "imgui_internal.h"

#include "EditorState.h"

bool UIEventLayer::handleEvent(const Event& e)
{
    if (!m_isEnabled)
        return false;

    bool isHandled = false;

    // Is it keyboard event?
    if (e.type() == EventType::KeyPressed || e.type() == EventType::KeyReleased)
    {
        const ImGuiIO& io = ImGui::GetIO();

        // UI needs it
        if (io.WantTextInput || io.WantCaptureKeyboard)
        {
            isHandled = true; //handled
        }
    }

    // is it mouse event?
    if (e.type() == EventType::MouseButtonPressed ||
        e.type() == EventType::MouseButtonReleased ||
        e.type() == EventType::MouseMoved ||
        e.type() == EventType::MouseWheel)
    {
        ImGuiWindow* hoveredWindow = ImGui::GetCurrentContext()->HoveredWindow;
        ImGuiWindow* sceneViewWindow = ImGui::FindWindowByName("View");

        EditorState::Instance().isMouseInSceneView = hoveredWindow != nullptr
            && sceneViewWindow != nullptr
            && (hoveredWindow == sceneViewWindow
                || hoveredWindow->RootWindow == sceneViewWindow);

        // mouse is not in scene view
        if (!EditorState::Instance().isMouseInSceneView)
        {
            const ImGuiIO& io = ImGui::GetIO();
            if (io.WantCaptureMouse)
            {
                isHandled = true; //handled
            }
        }
    }

    return isHandled;
}