#include "core/Application.h"

#include "core/Logger.h"
#include "core/Engine.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "systems/Skybox.h"
#include "ui/Input.h"
#include "ui/Mouse.h"
#include "ui/Keyboard.h"
#include "geometry/Box.h"
#include "geometry/Quad.h"

Context* Application::getContext() const
{
	return Engine::get()->getContext();
}

void Application::skybox(std::shared_ptr<Skybox> skybox)
{
	//auto context = Engine::get()->getContext();
	//context->getActiveScene()->setSkybox(skybox);
}

void Application::postProcess(ResourceWrapper<Shader> shader)
{
	auto context = Engine::get()->getContext();
	context->getActiveScene()->setPostProcessShader(shader);
}

bool Application::mouse_leftButtonPressed()
{
	return Engine::get()->getInput()->getMouse()->getButtonPressed(Mouse::MouseButton::LeftMousebutton);
}
bool Application::mouse_rightButtonPressed()
{
	return Engine::get()->getInput()->getMouse()->getButtonPressed(Mouse::MouseButton::RightMousebutton);
}
bool Application::mouse_middleButtonPressed()
{
	return Engine::get()->getInput()->getMouse()->getButtonPressed(Mouse::MouseButton::MiddleMousebutton);
}

int Application::keyboard_getKeyState(Keyboard::Key code)
{
	return Engine::get()->getInput()->getKeyboard()->getKeyState(code);
}

