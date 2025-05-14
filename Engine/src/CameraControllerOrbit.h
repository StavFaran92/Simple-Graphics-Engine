#pragma once

#include <glm/glm.hpp>

#include "ICameraController.h"
#include "Component.h"

class CameraControllerOrbit : public ICameraController
{
public:
	void onCreate(Entity& e) override;

private:
	enum class ControllerState
	{
		IDLE,
		ROTATE,
		TRANSFORM
	};

	ControllerState m_state = ControllerState::IDLE;

	float m_distance = 0;
	float m_angleX = 0;
	float m_angleY = 0;

	float m_turnSpeed = 0.5f;
	float m_movementSpeed = 1.f;

	glm::vec3 m_right;

	CameraComponent* m_cameraComponent = nullptr;
	Transformation* m_cameraTransform = nullptr;

	uint64_t m_eventHandler;
};