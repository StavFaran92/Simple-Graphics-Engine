#pragma once
#include "ScriptableEntity.h"

#include "Core.h"

#include "Keyboard.h"
#include <glm/glm.hpp>

class FlyCamera : public ScriptableEntity
{
public:
	FlyCamera(Entity camera, float yaw, float pitch, float moveSpeed, float turnSpeed);

	void keyControl(double deltaTime);
	void OnMouseMotion(float xChange, float yChange);
	void OnMousePressed(SDL_MouseButtonEvent& e);
	void OnMouseReleased(SDL_MouseButtonEvent& e);

	//inline glm::mat4 getView() override { return glm::lookAt(m_position, m_position + m_front, m_up); }
	//inline glm::vec3 getPosition() override { return m_position; }

	void onUpdate(float deltaTime) override;

	void onEvent(SDL_Event e) override;

	~FlyCamera();
private:
	void calculateOrientation();
private:
	glm::vec3 m_front;
	glm::vec3 m_up;
	glm::vec3 m_right;
	glm::vec3 m_worldUp;

	float m_yaw;
	float m_pitch;

	float m_movementSpeed;
	float m_turnSpeed;

	Entity m_camera;
};
