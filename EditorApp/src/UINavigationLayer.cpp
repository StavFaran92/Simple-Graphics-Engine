#include "UINavigationLayer.h"

#include "EditorState.h"

extern Entity g_editorCamera;

bool UINavigationLayer::handleEvent(SDL_Event e)
{
	if (!m_isEnabled)
		return false;

	//auto& camComponent = g_editorCamera.getComponent<CameraComponent>();
	auto& nsc = g_editorCamera.getComponent<NativeScriptComponent>();
	return nsc.script->onEvent(e);
}