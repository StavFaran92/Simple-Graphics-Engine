#include "GameLayer.h"

#include "ScriptableEntity.h"
#include "Engine.h"
#include "EventSystem.h"

bool GameLayer::handleEvent(SDL_Event e)
{
    //Engine::get()->getInput()->getMouse()->onEvent(e);
    //Engine::get()->getInput()->getKeyboard()->onEvent(e);

    Engine::get()->getEventSystem()->dispatch(e);

    //for (auto& listener: m_listeners) 
    //{
    //    listener->onEvent(e);
    //}
    return false;
}

void GameLayer::subscribe(std::shared_ptr<ScriptableEntity> e)
{
    m_listeners.insert(e);
}

void GameLayer::unsubscribe(std::shared_ptr<ScriptableEntity> e)
{
    m_listeners.erase(e); // todo check
}
