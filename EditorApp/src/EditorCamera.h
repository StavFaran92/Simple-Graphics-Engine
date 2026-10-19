#pragma once

#include "sge.h"

#include <cereal/types/polymorphic.hpp>
#include <cereal/cereal.hpp>


class EditorCamera : public ScriptableEntity
{
public:
	enum class CameraState
	{
		FreeLook,
		Orbit
	};
public:
	EditorCamera();
	~EditorCamera() = default;

	void onCreate() override;
	void onUpdate(float deltaTime) override;
	bool onEvent(const Event& e) override;

	void lock();
	void unlock();

	// Moves the camera to position, facing target
	void lookAt(const glm::vec3& position, const glm::vec3& target);
private:
	bool m_isLocked = false;

	CameraState m_camState = CameraState::FreeLook;

	std::shared_ptr<CameraControllerFreeLook> m_cameraController;

	CameraComponent* m_cameraComponent = nullptr;

};

CEREAL_REGISTER_TYPE(EditorCamera);
CEREAL_REGISTER_POLYMORPHIC_RELATION(ScriptableEntity, EditorCamera)