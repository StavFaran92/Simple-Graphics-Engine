#pragma once

#include <memory>

#include "core/Event.h"

class Entity;

class ICameraController
{
public:
	virtual void onCreate(Entity& e) = 0;
	virtual void onUpdate(float deltaTime) = 0;
	virtual bool onEvent(const Event& e) = 0;
	virtual ~ICameraController() = default;
};