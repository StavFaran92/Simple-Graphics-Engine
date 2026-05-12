#include "core/EventParser.h"
#include "core/KeyboardEvents.h"
#include "core/MouseEvents.h"
#include "core/WindowEvents.h"
#include "core/CoreEvents.h"

std::unique_ptr<Event> EventParser::parseSDLEvent(SDL_Event e)
{
    switch (e.type)
    {
    case SDL_KEYDOWN:
    {
        auto ev = std::make_unique<KeyPressedEvent>();
        ev->keysym = static_cast<KeyCode>(e.key.keysym.scancode);
        ev->repeat = e.key.repeat != 0;
        ev->state  = KeyState::Pressed;
        return ev;
    }
    case SDL_KEYUP:
    {
        auto ev = std::make_unique<KeyReleasedEvent>();
        ev->keysym = static_cast<KeyCode>(e.key.keysym.scancode);
        ev->repeat = e.key.repeat != 0;
        ev->state  = KeyState::Released;
        return ev;
    }
    case SDL_MOUSEBUTTONDOWN:
    {
        auto ev = std::make_unique<MouseButtonPressedEvent>();
        ev->button = static_cast<MouseButton>(e.button.button);
        ev->x      = e.button.x;
        ev->y      = e.button.y;
        ev->clicks = e.button.clicks;
        return ev;
    }
    case SDL_MOUSEBUTTONUP:
    {
        auto ev = std::make_unique<MouseButtonReleasedEvent>();
        ev->button = static_cast<MouseButton>(e.button.button);
        ev->x      = e.button.x;
        ev->y      = e.button.y;
        ev->clicks = e.button.clicks;
        return ev;
    }
    case SDL_MOUSEMOTION:
    {
        auto ev = std::make_unique<MouseMovedEvent>();
        ev->x    = e.motion.x;
        ev->y    = e.motion.y;
        ev->xrel = e.motion.xrel;
        ev->yrel = e.motion.yrel;
        return ev;
    }
    case SDL_MOUSEWHEEL:
    {
        auto ev = std::make_unique<MouseWheelEvent>();
        ev->x = e.wheel.x;
        ev->y = e.wheel.y;
        return ev;
    }
    case SDL_WINDOWEVENT:
    {
        if (e.window.event == SDL_WINDOWEVENT_RESIZED ||
            e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        {
            auto ev = std::make_unique<WindowResizedEvent>();
            ev->width  = e.window.data1;
            ev->height = e.window.data2;
            return ev;
        }
        if (e.window.event == SDL_WINDOWEVENT_CLOSE)
        {
            return std::make_unique<WindowClosedEvent>();
        }
        return nullptr;
    }
    case SDL_QUIT:
        return std::make_unique<QuitAppEvent>();

    default:
        return nullptr;
    }
}
