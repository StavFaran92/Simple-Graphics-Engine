#include "systems/ProfilerSystem.h"

#include <algorithm>

void ProfilerSystem::recordSample(const std::string& name, float milliseconds)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	auto& history = m_histories[name];
	history.samples[history.offset] = milliseconds;
	history.offset = (history.offset + 1) % kHistorySize;
	history.count = std::min(history.count + 1, kHistorySize);
	history.latest = milliseconds;
}

std::unordered_map<std::string, ProfilerSystem::ScopeHistory> ProfilerSystem::getSnapshot() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_histories;
}
