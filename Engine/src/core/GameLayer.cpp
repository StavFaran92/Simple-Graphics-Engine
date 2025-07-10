#include "core/GameLayer.h"

#include "ScriptableEntity.h"
#include "Engine.h"

bool GameLayer::handleEvent(SDL_Event e)
{
    if (!m_isEnabled) 
        return false;

    auto iter = m_listeners.find((SDL_EventType)e.type);
    if (iter != m_listeners.end())
    {
        for (auto& ec : iter->second)
        {
            ec.func(e);
        }
    }

    return false;
}