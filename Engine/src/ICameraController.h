#pragma once

#include <memory>

#include "SDL2/SDL.h"

class Entity;

class ICameraController
{
public:
	virtual void onCreate(Entity& e) = 0;
	virtual void onUpdate(float deltaTime) = 0;
	virtual void onEvent(SDL_Event e) = 0;
	virtual ~ICameraController() = default;
};