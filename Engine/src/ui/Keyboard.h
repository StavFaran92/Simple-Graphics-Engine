#pragma once

#include <iostream>
#include <functional>
#include "core/Core.h"

#include "core/EventLayer.h"

class EngineAPI Keyboard
{
public:

	enum Key : int
	{
		SCANCODE_UNKNOWN = 0,
		SCANCODE_A = 4,
		SCANCODE_B = 5,
		SCANCODE_C = 6,
		SCANCODE_D = 7,
		SCANCODE_E = 8,
		SCANCODE_F = 9,
		SCANCODE_G = 10,
		SCANCODE_H = 11,
		SCANCODE_I = 12,
		SCANCODE_J = 13,
		SCANCODE_K = 14,
		SCANCODE_L = 15,
		SCANCODE_M = 16,
		SCANCODE_N = 17,
		SCANCODE_O = 18,
		SCANCODE_P = 19,
		SCANCODE_Q = 20,
		SCANCODE_R = 21,
		SCANCODE_S = 22,
		SCANCODE_T = 23,
		SCANCODE_U = 24,
		SCANCODE_V = 25,
		SCANCODE_W = 26,
		SCANCODE_X = 27,
		SCANCODE_Y = 28,
		SCANCODE_Z = 29,
		SCANCODE_1 = 30,
		SCANCODE_2 = 31,
		SCANCODE_3 = 32,
		SCANCODE_4 = 33,
		SCANCODE_5 = 34,
		SCANCODE_6 = 35,
		SCANCODE_7 = 36,
		SCANCODE_8 = 37,
		SCANCODE_9 = 38,
		SCANCODE_0 = 39,
		SCANCODE_RETURN = 40,
		SCANCODE_ESCAPE = 41,
		SCANCODE_BACKSPACE = 42,
		SCANCODE_TAB = 43,
		SCANCODE_SPACE = 44,
		SCANCODE_MINUS = 45,
		SCANCODE_EQUALS = 46,
		SCANCODE_LEFTBRACKET = 47,
		SCANCODE_RIGHTBRACKET = 48,
		SCANCODE_BACKSLASH = 49,
		SCANCODE_NONUSHASH = 50,
		SCANCODE_SEMICOLON = 51,
		SCANCODE_APOSTROPHE = 52,
		SCANCODE_GRAVE = 53, 
		SCANCODE_COMMA = 54,
		SCANCODE_PERIOD = 55,
		SCANCODE_SLASH = 56,
		SCANCODE_CAPSLOCK = 57,
		SCANCODE_F1 = 58,
		SCANCODE_F2 = 59,
		SCANCODE_F3 = 60,
		SCANCODE_F4 = 61,
		SCANCODE_F5 = 62,
		SCANCODE_F6 = 63,
		SCANCODE_F7 = 64,
		SCANCODE_F8 = 65,
		SCANCODE_F9 = 66,
		SCANCODE_F10 = 67,
		SCANCODE_F11 = 68,
		SCANCODE_F12 = 69,
		SCANCODE_PRINTSCREEN = 70,
		SCANCODE_SCROLLLOCK = 71,
		SCANCODE_PAUSE = 72,
		SCANCODE_INSERT = 73,
		SCANCODE_HOME = 74,
		SCANCODE_PAGEUP = 75,
		SCANCODE_DELETE = 76,
		SCANCODE_END = 77,
		SCANCODE_PAGEDOWN = 78,
		SCANCODE_RIGHT = 79,
		SCANCODE_LEFT = 80,
		SCANCODE_DOWN = 81,
		SCANCODE_UP = 82,
		SCANCODE_NUMLOCKCLEAR = 83,
		SCANCODE_KP_DIVIDE = 84,
		SCANCODE_KP_MULTIPLY = 85,
		SCANCODE_KP_MINUS = 86,
		SCANCODE_KP_PLUS = 87,
		SCANCODE_KP_ENTER = 88,
		SCANCODE_KP_1 = 89,
		SCANCODE_KP_2 = 90,
		SCANCODE_KP_3 = 91,
		SCANCODE_KP_4 = 92,
		SCANCODE_KP_5 = 93,
		SCANCODE_KP_6 = 94,
		SCANCODE_KP_7 = 95,
		SCANCODE_KP_8 = 96,
		SCANCODE_KP_9 = 97,
		SCANCODE_KP_0 = 98,
		SCANCODE_KP_PERIOD = 99,
		SCANCODE_APPLICATION = 101, /**< windows contextual menu, compose */
		SCANCODE_KP_EQUALS = 103,
		SCANCODE_F13 = 104,
		SCANCODE_F14 = 105,
		SCANCODE_F15 = 106,
		SCANCODE_F16 = 107,
		SCANCODE_F17 = 108,
		SCANCODE_F18 = 109,
		SCANCODE_F19 = 110,
		SCANCODE_F20 = 111,
		SCANCODE_F21 = 112,
		SCANCODE_F22 = 113,
		SCANCODE_F23 = 114,
		SCANCODE_F24 = 115,
		SCANCODE_EXECUTE = 116,
		SCANCODE_HELP = 117,    /**< AL Integrated Help Center */
		SCANCODE_MENU = 118,    /**< Menu (show menu) */
		SCANCODE_SELECT = 119,
		SCANCODE_STOP = 120,    /**< AC Stop */
		SCANCODE_AGAIN = 121,   /**< AC Redo/Repeat */
		SCANCODE_UNDO = 122,    /**< AC Undo */
		SCANCODE_CUT = 123,     /**< AC Cut */
		SCANCODE_COPY = 124,    /**< AC Copy */
		SCANCODE_PASTE = 125,   /**< AC Paste */
		SCANCODE_FIND = 126,    /**< AC Find */
		SCANCODE_MUTE = 127,
		SCANCODE_VOLUMEUP = 128,
		SCANCODE_VOLUMEDOWN = 129,
		SCANCODE_ALTERASE = 153,    /**< Erase-Eaze */
		SCANCODE_SYSREQ = 154,
		SCANCODE_CANCEL = 155,      /**< AC Cancel */
		SCANCODE_CLEAR = 156,
		SCANCODE_PRIOR = 157,
		SCANCODE_RETURN2 = 158,
		SCANCODE_SEPARATOR = 159,
		SCANCODE_OUT = 160,
		SCANCODE_OPER = 161,
		SCANCODE_CLEARAGAIN = 162,
		SCANCODE_CRSEL = 163,
		SCANCODE_EXSEL = 164,
		SCANCODE_LCTRL = 224,
		SCANCODE_LSHIFT = 225,
		SCANCODE_LALT = 226, /**< alt, option */
		SCANCODE_LGUI = 227, /**< windows, command (apple), meta */
		SCANCODE_RCTRL = 228,
		SCANCODE_RSHIFT = 229,
		SCANCODE_RALT = 230, /**< alt gr, option */
		SCANCODE_RGUI = 231, /**< windows, command (apple), meta */

	};
	enum KeyState : int
	{
		Invalid,
		Pressed,
		Released
	};
	struct KeyEvent
	{
		KeyState state = KeyState::Invalid;
		bool repeat = false;
		Key keysym = SCANCODE_UNKNOWN;
	};
	using KeyCallback = std::function<void(KeyEvent s)>;

	Keyboard();
	int getKeyState(Key code) const;
	void onKeyPressed(EventHandler handler, Key code, KeyCallback callback) const;
	void onKeyReleased(EventHandler handler, Key code, KeyCallback callback) const;

private:
	const uint8_t* m_keyboardState = nullptr;
	int m_length = 0;
};

class EngineAPI GameKeyboard : public Keyboard
{
public:
	GameKeyboard();

private:
	EventHandler gameHandler;
};
