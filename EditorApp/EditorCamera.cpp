#include "EditorCamera.h"

#include <gl/glew.h>

#include "ApplicationConstants.h"
#include "Engine.h"
#include "Context.h"
#include "Scene.h"
#include "Transformation.h"
#include "CameraControllerOrbit.h"
#include "CameraControllerFreeLook.h"
#include "EventSystem.h"

#include "EditorState.h"

#include "Logger.h"

EditorCamera::EditorCamera()
{
	m_cameraController = std::make_shared<CameraControllerFreeLook>();

	auto eventSystem = Engine::get()->getEventSystem();

	m_eventHandler = eventSystem->bindToLayer("GameLayer"); // TODO fix

	eventSystem->subscribe(m_eventHandler, SDL_MOUSEMOTION, this);
	eventSystem->subscribe(m_eventHandler, SDL_MOUSEBUTTONDOWN, this);
	eventSystem->subscribe(m_eventHandler, SDL_MOUSEBUTTONUP, this);
	eventSystem->subscribe(m_eventHandler, SDL_MOUSEWHEEL, this);
}

void EditorCamera::onCreate()
{
	m_cameraController->onCreate(entity);
}

void EditorCamera::onUpdate(float deltaTime)
{
	m_cameraController->onUpdate(deltaTime);
}

void EditorCamera::onEvent(SDL_Event e)
{
	m_cameraController->onEvent(e);
}