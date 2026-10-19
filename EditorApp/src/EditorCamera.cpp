#include "EditorCamera.h"

#include <gl/glew.h>

#include "sge.h"

#include "EditorState.h"

bool shouldSceneViewGetKeyboardInner()
{
	const ImGuiIO& io = ImGui::GetIO();

	bool uiFocused = io.WantTextInput;

	// If UI needs keyboard -> block scene
	if (uiFocused)
		return false;

	// Otherwise: Override ImGui's WantCaptureKeyboard
	return true;
}

EditorCamera::EditorCamera()
{
	m_cameraController = std::make_shared<CameraControllerFreeLook>();
}

void EditorCamera::onCreate()
{
	m_cameraController->onCreate(entity);
}

void EditorCamera::onUpdate(float deltaTime)
{
	if (m_isLocked)
		return;

	if (!shouldSceneViewGetKeyboardInner())
		return;

	m_cameraController->onUpdate(deltaTime);
}

bool EditorCamera::onEvent(const Event& e)
{
	if (m_isLocked)
		return false;

	if (!EditorState::Instance().isMouseInSceneView) 
		return false;

	m_cameraController->onEvent(e);

	return false;
}

void EditorCamera::lock()
{
	m_isLocked = true;
}

void EditorCamera::unlock()
{
	m_isLocked = false;
}

void EditorCamera::lookAt(const glm::vec3& position, const glm::vec3& target)
{
	// World values are cached until update(): refresh so lookAt sees the new position, and callers the new rotation
	auto& transform = entity.getComponent<Transformation>();
	transform.setWorldPosition(position);
	transform.update();
	m_cameraController->lookAt(target);
	transform.update();
}
