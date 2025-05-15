#include "GameLayer.h"

#include "ScriptableEntity.h"
#include "Engine.h"

bool GameLayer::handleEvent(SDL_Event e)
{
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

void GameLayer::subscribe(SDL_EventType eventType, const EventCallback& callback)
{
    m_listeners[eventType].push_back(callback);
}

void GameLayer::unsubscribe(EventHandler handler, SDL_EventType eventType)
{
    auto iter = m_listeners.find(eventType);
    if (iter != m_listeners.end())
    {
        std::vector<EventCallback>& eventSubscribers = iter->second;
        for (int i=0; i<eventSubscribers.size(); i++)
        {
            if (eventSubscribers[i].handler == handler)
            {
                eventSubscribers.erase(eventSubscribers.begin() + i);
                return;
            }
        }
    }
}

