#include "FlyCamera.h"

#include "EventSystem.h"
#include <gl/glew.h>
#include "Logger.h"
#include "Engine.h"
#include "Input.h"
#include "Component.h"
#include "Transformation.h"

FlyCamera::FlyCamera(Entity camera, float yaw, float pitch, float moveSpeed, float turnSpeed)
	:m_camera(camera),
	m_worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
	m_yaw(yaw),
	m_pitch(pitch),
	m_front(glm::vec3(0.0f, 0.0f, -1.0f)),
	m_movementSpeed(moveSpeed),
	m_turnSpeed(turnSpeed)
{
	

	//auto eventSystem = Engine::get()->getEventSystem();

	//eventSystem->subscribe(SDL_MOUSEMOTION, [this](SDL_Event e) {
	//	OnMouseMotion(e.motion.xrel, e.motion.yrel);
	//});
}

void FlyCamera::OnMouseMotion(float xChange, float yChange)
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

void FlyCamera::OnMousePressed(SDL_MouseButtonEvent& e)
{
}

void FlyCamera::OnMouseReleased(SDL_MouseButtonEvent& e)
{
}

void FlyCamera::calculateOrientation()
{
	m_front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_front.y = sin(glm::radians(m_pitch));
	m_front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

	m_front = glm::normalize(m_front);

	m_right = glm::normalize(glm::cross(m_front, m_worldUp));
	m_up = glm::normalize(glm::cross(m_right, m_front));

	auto& camComponent = m_camera.getComponent<CameraComponent>();
	auto& camTransform = m_camera.getComponent<Transformation>();



	// Create quaternions for pitch and yaw
	glm::quat pitchQuat = glm::angleAxis(Constants::PI - m_pitch * Constants::toRadians, glm::vec3(1, 0, 0));
	glm::quat yawQuat = glm::angleAxis(Constants::PI - m_yaw * Constants::toRadians, glm::vec3(0, 1, 0));

	// Combine the quaternions
	glm::quat combinedQuat = yawQuat * pitchQuat;

	camTransform.setLocalRotation(combinedQuat);

	camComponent.up = m_up;
	camComponent.center = camTransform.getWorldPosition() + m_front;
	camComponent.front = m_front;
	camComponent.right = m_right;
}

void FlyCamera::onUpdate(float deltaTime)
{
}

void FlyCamera::onEvent(SDL_Event e)
{
	if (e.type == SDL_MOUSEMOTION)
	{
		OnMouseMotion(e.motion.xrel, e.motion.yrel);
	}
}

FlyCamera::~FlyCamera()
{}