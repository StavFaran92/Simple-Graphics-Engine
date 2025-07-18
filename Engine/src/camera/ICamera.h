#pragma once

#include <glm/glm.hpp>

#include "Core.h"
#include "ScriptableEntity.h"

class EngineAPI ICamera : public ScriptableEntity
{
public:
	ICamera() = default;

	virtual glm::mat4 getView() = 0;
	virtual glm::vec3 getPosition() = 0;
	virtual void lookAt(float x, float y, float z) = 0;
	virtual void setPosition(float x, float y, float z) = 0;

	virtual ~ICamera() = default;
};
