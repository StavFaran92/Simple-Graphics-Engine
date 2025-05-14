#include "UIEventLayer.h"

bool UIEventLayer::handleEvent(SDL_Event e)
{
    return false;
}

void UIEventLayer::subscribe(SDL_EventType eventType, const std::function<void(SDL_Event e)>& callback)
{
}
