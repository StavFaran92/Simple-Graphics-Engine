#include "core/Application.h"

#include "core/Logger.h"
#include "core/Engine.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "ui/Input.h"
#include "ui/Mouse.h"
#include "ui/Keyboard.h"
#include "geometry/Box.h"
#include "geometry/Quad.h"

Context* Application::getContext() const
{
	return Engine::get()->getContext();
}

bool Application::mouse_leftButtonPressed()
{
	return Engine::get()->getInput()->getMouse()->getButtonPressed(MouseButton::MOUSE_BUTTON_LEFT);
}
bool Application::mouse_rightButtonPressed()
{
	return Engine::get()->getInput()->getMouse()->getButtonPressed(MouseButton::MOUSE_BUTTON_RIGHT);
}
bool Application::mouse_middleButtonPressed()
{
	return Engine::get()->getInput()->getMouse()->getButtonPressed(MouseButton::MOUSE_BUTTON_MIDDLE);
}

int Application::keyboard_getKeyState(KeyCode code)
{
	return Engine::get()->getInput()->getKeyboard()->getKeyState(code);
}

