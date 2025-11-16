#include "EditorState.h"

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

WorkingDirectory& EditorState::getWorkingDir()
{
    return cwd;
}