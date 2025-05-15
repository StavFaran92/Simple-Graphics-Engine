#include "UIEventLayer.h"

bool UIEventLayer::handleEvent(SDL_Event e)
{
    return false;
}

void UIEventLayer::subscribe(SDL_EventType eventType, const EventCallback& callback)
{
}

void UIEventLayer::unsubscribe(EventHandler handler, SDL_EventType eventType)
{
}
