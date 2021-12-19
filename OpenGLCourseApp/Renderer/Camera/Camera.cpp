#include "Camera.h"

Camera::Camera(glm::vec3 startPosition, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed)
	:m_position(startPosition), 
	m_worldUp(glm::vec3(0.0f, 1.0f, 0.0f)), 
	m_yaw(startYaw), 
	m_pitch(startPitch),
	m_front(glm::vec3(0.0f, 0.0f, -1.0f)), 
	m_movementSpeed(startMoveSpeed), 
	m_turnSpeed(startTurnSpeed)
{
	keyboard = std::make_shared<Keyboard>();
}

void Camera::keyControl( double deltaTime)
{
	GLfloat velocity =  m_movementSpeed * deltaTime;	

	if (keyboard->getKeyState(SDL_SCANCODE_W))
	{
		m_position += m_front * velocity;
	}

	if (keyboard->getKeyState(SDL_SCANCODE_S))
	{
		m_position -= m_front * velocity;
	}

	if (keyboard->getKeyState(SDL_SCANCODE_A))
	{
		m_position -= m_right * velocity;
	}

	if (keyboard->getKeyState(SDL_SCANCODE_D))
	{
		m_position += m_right * velocity;
	}
}

void Camera::mouseControl(GLfloat xChange, GLfloat yChange)
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

void Camera::calculateOrientation()
{
	m_front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_front.y = sin(glm::radians(m_pitch));
	m_front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

	m_front = glm::normalize(m_front);

	m_right = glm::normalize(glm::cross(m_front, m_worldUp));
	m_up = glm::normalize(glm::cross(m_right, m_front));
}

void Camera::update(float deltaTime)
{
	keyControl(deltaTime);
}

Camera::~Camera()
{}