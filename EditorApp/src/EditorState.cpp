#include "EditorState.h"

#include "Dialogs/DialogBase.h"

void EditorState::init()
{
    cwd.m_path = ScopedPath::ContentPath();
}

bool EditorState::getState(const std::string& state)
{
    if (auto it = m_states.find(state); it != m_states.end())
        return it->second;

    return false;
}

bool EditorState::setState(const std::string& state, bool enabled)
{
    bool old = getState(state);
    m_states[state] = enabled;
    return old;
}

void EditorState::addDialogDisplay(DialogBase* dialog)
{
    m_dialogs.push_back(dialog);
}

void EditorState::displayDialogs()
{
    for (auto dialog : m_dialogs)
    {
        dialog->display();
    }
}

WorkingDirectory& EditorState::getWorkingDir()
{
    return cwd;
}