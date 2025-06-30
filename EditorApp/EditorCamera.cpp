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
}

void EditorCamera::onCreate()
{
	auto eventSystem = Engine::get()->getEventSystem();

	eventSystem->subscribe(eventHandler, SDL_MOUSEMOTION, this);
	eventSystem->subscribe(eventHandler, SDL_MOUSEBUTTONDOWN, this);
	eventSystem->subscribe(eventHandler, SDL_MOUSEBUTTONUP, this);
	eventSystem->subscribe(eventHandler, SDL_MOUSEWHEEL, this);

	m_cameraController->onCreate(entity);
}

void EditorCamera::onUpdate(float deltaTime)
{
	if (m_isLocked)
		return;

	m_cameraController->onUpdate(deltaTime);
}

void EditorCamera::onEvent(SDL_Event e)
{
	if (m_isLocked)
		return;

	if (!EditorState::Instance().isMouseInSceneView) return;

	m_cameraController->onEvent(e);
}

void EditorCamera::lock()
{
	m_isLocked = true;
}

void EditorCamera::unlock()
{
	m_isLocked = false;
}
