#pragma once

#include "sge.h"

#include <cereal/types/polymorphic.hpp>
#include <cereal/cereal.hpp>

class CameraScript : public ScriptableEntity
{
	virtual void onCreate() override
	{
		Engine::get()->getWindow()->lockMouse();

		auto eventSystem = Engine::get()->getEventSystem();

		m_camera = entity;

		eventSystem->subscribe(eventHandler, SDL_MOUSEMOTION, [this](SDL_Event e) {
			OnMouseMotion(e.motion.xrel, e.motion.yrel);
			});
	}

	virtual void onDestroy() override
	{
		Engine::get()->getWindow()->unlockMouse();

		auto eventSystem = Engine::get()->getEventSystem();

		//eventSystem->removeEventListener(m_handler); // TODO fix
	}

	virtual void onUpdate(float deltaTime) {
	};

	void OnMouseMotion(float xChange, float yChange)
	{
		xChange *= m_turnSpeed;
		yChange *= m_turnSpeed;

		m_yaw += xChange;
		m_pitch -= yChange;

		if (m_pitch > 89.0f)
		{
			m_pitch = 89.0f;
		}

		if (m_pitch < -89.0f)
		{
			m_pitch = -89.0f;
		}

		calculateOrientation();
	}

	void calculateOrientation()
	{
		auto& camTransform = m_camera.getComponent<Transformation>();

		// Create quaternions for pitch and yaw
		glm::quat pitchQuat = glm::angleAxis(Constants::PI + m_pitch * Constants::toRadians + Constants::PI / 2, glm::vec3(1, 0, 0));
		glm::quat yawQuat = glm::angleAxis(Constants::PI - m_yaw * Constants::toRadians + Constants::PI / 2, glm::vec3(0, 1, 0));

		// Combine the quaternions
		glm::quat combinedQuat = yawQuat * pitchQuat;

		camTransform.setLocalRotation(combinedQuat);
	}

	uint64_t m_handler;

	float m_yaw = 0;
	float m_pitch = 0;

	float m_movementSpeed = 3;
	float m_turnSpeed = 1;

	Entity m_camera;
};

CEREAL_REGISTER_TYPE(CameraScript);
CEREAL_REGISTER_POLYMORPHIC_RELATION(ScriptableEntity, CameraScript)