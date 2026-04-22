#pragma once

#include <chrono>

#include "core/Core.h"
#include "systems/SubSystem.h"

class EngineAPI TimeManager : public SubSystem
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
	long long getCurrentTime(Duration d = Duration::MicroSeconds) const;
private:
	std::chrono::steady_clock::time_point m_beginTime;
};