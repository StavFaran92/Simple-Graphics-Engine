#include "GameLayer.h"

#include "ScriptableEntity.h"
#include "Engine.h"

bool GameLayer::handleEvent(SDL_Event e)
{
    if (!m_isEnabled) 
        return false;

    auto iter = m_listeners.find((SDL_EventType)e.type);
    if (iter != m_listeners.end())
    {
        for (auto& s : iter->second)
        {
            s->onEvent(e);
        }
    }

    return false;
}

void GameLayer::subscribe(SDL_EventType eventType, Subscriber* s)
{
    m_listeners[eventType].push_back(s);
}

void GameLayer::unsubscribe(SDL_EventType eventType, Subscriber* s)
{
    auto iter = m_listeners.find(eventType);
    if (iter != m_listeners.end())
    {
        std::vector<Subscriber*>& eventSubscribers = iter->second;
        for (int i=0; i<eventSubscribers.size(); i++)
        {
            if (eventSubscribers[i] == s)
            {
                eventSubscribers.erase(eventSubscribers.begin() + i);
                return;
            }
        }
    }
}

