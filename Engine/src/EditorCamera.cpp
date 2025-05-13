#include "EditorCamera.h"

#include <gl/glew.h>

#include "ApplicationConstants.h"
#include "Engine.h"
#include "Context.h"
#include "Scene.h"
#include "EventSystem.h"
#include "Transformation.h"
#include "CameraControllerOrbit.h"
#include "CameraControllerFreeLook.h"

#include "Logger.h"

EditorCamera::EditorCamera()
{
	m_cameraController = std::make_shared<CameraControllerFreeLook>();
}

void EditorCamera::onCreate()
{
	m_cameraController->onCreate(entity, m_eventSystem);
}

void EditorCamera::onUpdate(float deltaTime)
{
	m_cameraController->onUpdate(deltaTime);
}