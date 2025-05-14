#pragma once

#include <memory>

class Entity;

class ICameraController
{
public:
	virtual void onCreate(Entity& e) = 0;
	virtual void onUpdate(float deltaTime) = 0;
	virtual ~ICameraController() = default;
};