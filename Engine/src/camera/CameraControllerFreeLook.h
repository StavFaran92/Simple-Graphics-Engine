#pragma once

#include <glm/glm.hpp>

#include "camera/ICameraController.h"
#include "component/CameraComponent.h"
#include "core/Subscriber.h"
#include "component/Transformation.h"

#include "ui/KeyCodes.h"

class EngineAPI CameraControllerFreeLook : public ICameraController
{
public:
	void onCreate(Entity& e) override;
	void onUpdate(float deltaTime) override;
	bool onEvent(const Event& e) override;

private:
	enum class ControllerState
	{
		IDLE,
		ROTATE,
		TRANSFORM
	};
private:
	bool m_isLocked = true;

	float m_yaw = 0;
	float m_pitch = 0;

	glm::vec3 m_velocityF;
	glm::vec3 m_velocityR;
	glm::vec3 m_velocityU;

	float velocity = 50.f;
	//glm::vec3 m_up = { 0,1,0 };
	//glm::vec3 m_right;
	//glm::vec3 m_front;

	float m_turnSpeed = 10.f;
	float m_distance = 0;
	float m_movementSpeed = 10.f;
	float m_minMovementSpeed = 0.1f;
	float m_maxMovementSpeed = 500.f;
	float m_speedScrollFactor = 1.2f; // speed multiplier per wheel notch in movement mode

	ControllerState m_state = ControllerState::IDLE;

	CameraComponent* m_cameraComponent = nullptr;
	Transformation* m_cameraTransform = nullptr;
};