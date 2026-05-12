#pragma once

#include "core/Event.h"

class Subscriber
{
public:
	virtual bool onEvent(const Event& e) { return false; };
};