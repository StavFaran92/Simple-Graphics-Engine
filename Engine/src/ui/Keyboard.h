#pragma once

#include <iostream>
#include <functional>
#include "core/Core.h"

#include "core/EventLayer.h"
#include "ui/KeyCodes.h"
#include "systems/SubSystem.h"
#include "core/KeyboardEvents.h"

class EngineAPI Keyboard
{
public:
	using KeyPressedCallback = std::function<bool(KeyPressedEvent s)>;
	using KeyReleasedCallback = std::function<bool(KeyReleasedEvent s)>;

	Keyboard();
	int getKeyState(KeyCode code) const;
	void onKeyPressed(EventHandler handler, KeyCode code, std::function<bool(KeyPressedEvent s)> callback) const;
	void onKeyReleased(EventHandler handler, KeyCode code, std::function<bool(KeyReleasedEvent s)>  callback) const;

private:
	const uint8_t* m_keyboardState = nullptr;
	int m_length = 0;
};

class EngineAPI GameKeyboard : public Keyboard, public SubSystem
{
public:
	GameKeyboard();
	void onKeyPressed(KeyCode code, KeyPressedCallback callback) const;
	void onKeyReleased(KeyCode code, KeyReleasedCallback callback) const;

private:
	EventHandler gameHandler;
};
