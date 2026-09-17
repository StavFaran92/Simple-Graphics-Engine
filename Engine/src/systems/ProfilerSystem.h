#pragma once

#include <array>
#include <mutex>
#include <string>
#include <unordered_map>

#include "core/Core.h"
#include "systems/SubSystem.h"

// Lightweight in-memory history of ScopeTimer samples, keyed by scope name.
// Populated by ScopeTimer, read by debug/editor UI to draw graphs.
class EngineAPI ProfilerSystem : public SubSystem
{
public:
	static constexpr int kHistorySize = 120;

	struct ScopeHistory
	{
		std::array<float, kHistorySize> samples{};
		int offset = 0; // index the next sample will be written to
		int count = 0;  // number of valid samples (caps at kHistorySize)
		float latest = 0.0f;
	};

	ProfilerSystem() = default;

	void recordSample(const std::string& name, float milliseconds);

	// Returns a copy of every scope's history, safe to read/iterate without locking.
	std::unordered_map<std::string, ScopeHistory> getSnapshot() const;

private:
	mutable std::mutex m_mutex;
	std::unordered_map<std::string, ScopeHistory> m_histories;
};
