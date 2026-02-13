#include "UIEditorToolsEventLayer.h"

#include "EditorState.h"

bool UIEditorToolsEventLayer::handleEvent(SDL_Event e)
{
    if (!m_isEnabled)
        return false;

    bool isHandled = false;

    auto activeEditorTool = EditorState::Instance().getActiveEditorTool();
    if (activeEditorTool)
    {
        isHandled |= activeEditorTool->onEvent(e);
    }

    return isHandled;
}