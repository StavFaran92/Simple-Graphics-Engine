#pragma once

#include "sge.h"
#include "ICameraController.h"

#include <cereal/types/polymorphic.hpp>
#include <cereal/cereal.hpp>
#include "CameraControllerFreeLook.h"

class CameraScript : public ScriptableEntity
{
public:
	enum class CameraState
	{
		FreeLook,
		Orbit
	};
public:
	CameraScript()
	{
		m_cameraController = std::make_shared<CameraControllerFreeLook>();
	}
	~CameraScript() = default;

	void onCreate() override
	{
		auto eventSystem = Engine::get()->getEventSystem();

		eventHandler = eventSystem->bindToLayer("GameLayer");

		eventSystem->subscribe(eventHandler, SDL_MOUSEMOTION, this);
		eventSystem->subscribe(eventHandler, SDL_MOUSEBUTTONDOWN, this);
		eventSystem->subscribe(eventHandler, SDL_MOUSEBUTTONUP, this);
		eventSystem->subscribe(eventHandler, SDL_MOUSEWHEEL, this);

		m_cameraController->onCreate(entity);
	}
	void onUpdate(float deltaTime) override
	{
		m_cameraController->onUpdate(deltaTime);
	}
	void onEvent(SDL_Event e)
	{
		m_cameraController->onEvent(e);
	}
private:
	bool m_isLocked = true;

	CameraState m_camState = CameraState::FreeLook;

	std::shared_ptr<ICameraController> m_cameraController;

	CameraComponent* m_cameraComponent = nullptr;

};

CEREAL_REGISTER_TYPE(CameraScript);
CEREAL_REGISTER_POLYMORPHIC_RELATION(ScriptableEntity, CameraScript)