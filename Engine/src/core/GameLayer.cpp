#include "core/GameLayer.h"

#include "component/ScriptableEntity.h"
#include "core/Engine.h"
#include "core/Logger.h"

// todo remove 
#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "core/GameEventSystem.h"

bool GameLayer::handleEvent(SDL_Event e)
{
    if (!m_isEnabled) 
        return false;

    auto iter = m_listeners.find((SDL_EventType)e.type);
    if (iter != m_listeners.end())
    {
        for (auto& ec : iter->second)
        {
            try
            {
                ec.func(e);
            }
            catch (const std::exception& e)
            {
                logError("Exception occured: {}", e.what());
            }
        }
    }

    Engine::get()->getSubSystem<GameEventSystem>()->dispatch(e);

    return false;
}