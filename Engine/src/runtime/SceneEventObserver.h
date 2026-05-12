#pragma once

#include "core/Subscriber.h"

class SceneEventObserver : public Subscriber
{
public:
	bool onEvent(const Event& e) override;

	
};