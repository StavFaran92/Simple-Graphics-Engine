#include "core/Application.h"

#include "systems/Logger.h"
#include "core/Engine.h"
#include "Context.h"
#include "Scene.h"
#include "Skybox.h"
#include "Input.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "Box.h"
#include "Quad.h"

Context* Application::getContext() const
{
	return Engine::get()->getContext();
}

void Application::skybox(std::shared_ptr<Skybox> skybox)
{
	//auto context = Engine::get()->getContext();
	//context->getActiveScene()->setSkybox(skybox);
}

void Application::postProcess(Resource<Shader> shader)
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

int Application::keyboard_getKeyState(SDL_Scancode code)
{
	return Engine::get()->getInput()->getKeyboard()->getKeyState(code);
}

