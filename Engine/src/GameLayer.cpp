#include "GameLayer.h"

#include "ScriptableEntity.h"
#include "Engine.h"

GameLayer::GameLayer()
{
    m_eventSystem = std::make_shared<EventSystem>();
}

bool GameLayer::handleEvent(SDL_Event e)
{
    m_eventSystem->dispatch(e);

    return false;
}

std::shared_ptr<EventSystem> GameLayer::getEventSystem() const
{
    return m_eventSystem;
}

