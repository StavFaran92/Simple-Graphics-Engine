#pragma once

#include <chrono>

#include "Core.h"

class EngineAPI TimeManager
{
public:
	enum class Duration
	{
		Seconds,
		MilliSeconds,
		MicroSeconds,
		NanoSeconds
	};
	TimeManager();

	long long getElapsedTime(Duration d = Duration::MicroSeconds) const;
private:
	std::chrono::steady_clock::time_point m_beginTime;
};