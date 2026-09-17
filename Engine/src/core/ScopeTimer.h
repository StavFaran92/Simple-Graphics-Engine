#pragma once

#include <chrono>
#include <string>
#include <utility>

#include "core/Engine.h"
#include "systems/ProfilerSystem.h"

// Lightweight RAII timer: records how long the enclosing scope took (in ms) into
// ProfilerSystem once it's destroyed, so it can be graphed in the debug/editor UI.
// Usage: { ScopeTimer timer("MySection"); ...code to measure...; }
// Or:    SCOPE_TIMER("MySection");
class ScopeTimer
{
public:
	explicit ScopeTimer(std::string name)
		: m_name(std::move(name)), m_start(std::chrono::steady_clock::now())
	{
	}

	ScopeTimer(const ScopeTimer&) = delete;
	ScopeTimer& operator=(const ScopeTimer&) = delete;

	~ScopeTimer()
	{
		const auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_start).count();
		const float elapsedMs = static_cast<float>(elapsedUs) / 1000.0f;

		// Best-effort: ProfilerSystem may not be registered yet (e.g. very early startup).
		try
		{
			Engine::get()->getSubSystem<ProfilerSystem>()->recordSample(m_name, elapsedMs);
		}
		catch (...)
		{
		}
	}

private:
	std::string m_name;
	std::chrono::steady_clock::time_point m_start;
};

#define SCOPE_TIMER_CONCAT_INNER(a, b) a##b
#define SCOPE_TIMER_CONCAT(a, b) SCOPE_TIMER_CONCAT_INNER(a, b)
#define SCOPE_TIMER(name) ScopeTimer SCOPE_TIMER_CONCAT(scopeTimer_, __LINE__)(name)
