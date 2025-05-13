#pragma once

#include <memory>

class Entity;
class EventSystem;

class ICameraController
{
public:
	virtual void onCreate(Entity& e, std::shared_ptr<EventSystem> eventsystem) = 0;
	virtual void onUpdate(float deltaTime) = 0;
	virtual ~ICameraController() = default;
};