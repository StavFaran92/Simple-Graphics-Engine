#pragma once

class Entity;

class ICameraController
{
public:
	virtual void onCreate(Entity& e) = 0;
	virtual void calculateOrientation() = 0;
	virtual ~ICameraController() = default;
};