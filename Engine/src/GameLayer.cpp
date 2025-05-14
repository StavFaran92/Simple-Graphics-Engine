#include "GameLayer.h"

#include "ScriptableEntity.h"
#include "Engine.h"

bool GameLayer::handleEvent(SDL_Event e)
{
    auto iter = m_listeners.find((SDL_EventType)e.type);
    if (iter != m_listeners.end())
    {
        for (auto& c : iter->second)
        {
            c(e);
        }
    }

    return false;
}

void GameLayer::subscribe(SDL_EventType eventType, const std::function<void(SDL_Event e)>& callback)
{
    m_listeners[eventType].push_back(callback);
}

