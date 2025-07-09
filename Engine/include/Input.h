#pragma once

#include "core/Core.h"
#include "ui/Keyboard.h"
#include "ui/Mouse.h"

class EngineAPI Input 
{
public:
	Input();
	void init();
	const Keyboard* getKeyboard() const;
	Mouse* getMouse() const;

private:
	std::shared_ptr<Keyboard> m_keyboard;
	std::shared_ptr<Mouse> m_mouse;

};


