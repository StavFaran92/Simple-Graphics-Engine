#include "Input.h"

Input::Input()
{
}

void Input::init()
{
	m_keyboard = std::make_shared<Keyboard>();
	m_mouse = std::make_shared<Mouse>();
}

const Keyboard* Input::getKeyboard() const
{
	return m_keyboard.get();
}

Mouse* Input::getMouse() const
{
	return m_mouse.get();
}
