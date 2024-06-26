//#include "FlyCamera.h"
//
//#include "EventSystem.h"
//#include <gl/glew.h>
//#include "Logger.h"
//
//FlyCamera::FlyCamera(glm::vec3 startPosition, float startYaw, float startPitch, float startMoveSpeed, float startTurnSpeed)
//	:m_position(startPosition),
//	m_worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
//	m_yaw(startYaw),
//	m_pitch(startPitch),
//	m_front(glm::vec3(0.0f, 0.0f, -1.0f)),
//	m_movementSpeed(startMoveSpeed),
//	m_turnSpeed(startTurnSpeed)
//{
//	auto eventSystem = Engine::get()->getEventSystem();
//
//	eventSystem->addEventListener(SDL_MOUSEMOTION, [this](SDL_Event e) {
//		OnMouseMotion(e.motion.xrel, e.motion.yrel);
//	});
//}
//
//void FlyCamera::keyControl(double deltaTime)
//{
//	float velocity = m_movementSpeed * deltaTime;
//
//	if (Engine::get()->getInput()->getKeyboard()->getKeyState(SDL_SCANCODE_W))
//	{
//		m_position += m_front * velocity;
//	}
//
//	if (Engine::get()->getInput()->getKeyboard()->getKeyState(SDL_SCANCODE_S))
//	{
//		m_position -= m_front * velocity;
//	}
//
//	if (Engine::get()->getInput()->getKeyboard()->getKeyState(SDL_SCANCODE_A))
//	{
//		m_position -= m_right * velocity;
//	}
//
//	if (Engine::get()->getInput()->getKeyboard()->getKeyState(SDL_SCANCODE_D))
//	{
//		m_position += m_right * velocity;
//	}
//}
//
//void FlyCamera::OnMouseMotion(float xChange, float yChange)
//{
//	xChange *= m_turnSpeed;
//	yChange *= m_turnSpeed;
//
//	m_yaw += xChange;
//	m_pitch -= yChange;
//
//	if (m_pitch > 89.0f)
//	{
//		m_pitch = 89.0f;
//	}
//
//	if (m_pitch < -89.0f)
//	{
//		m_pitch = -89.0f;
//	}
//
//	calculateOrientation();
//}
//
//void FlyCamera::OnMousePressed(SDL_MouseButtonEvent& e)
//{
//}
//
//void FlyCamera::OnMouseReleased(SDL_MouseButtonEvent& e)
//{
//}
//
//void FlyCamera::calculateOrientation()
//{
//	m_front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
//	m_front.y = sin(glm::radians(m_pitch));
//	m_front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
//
//	m_front = glm::normalize(m_front);
//
//	m_right = glm::normalize(glm::cross(m_front, m_worldUp));
//	m_up = glm::normalize(glm::cross(m_right, m_front));
//}
//
//void FlyCamera::update(float deltaTime)
//{
//	keyControl(deltaTime);
//}
//
//FlyCamera::~FlyCamera()
//{}