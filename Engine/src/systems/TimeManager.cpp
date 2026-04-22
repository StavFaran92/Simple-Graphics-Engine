#include "systems/TimeManager.h"

#include "core/Engine.h"

TimeManager::TimeManager()
{
	Engine::get()->registerSubSystem<TimeManager>(this);
	m_beginTime = std::chrono::high_resolution_clock::now();
}

long long TimeManager::getElapsedTime(Duration d /*= Duration::MicroSeconds*/) const
{
	if (d == Duration::NanoSeconds)
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_beginTime).count();
	if(d == Duration::MicroSeconds)
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_beginTime).count();
	if(d == Duration::MilliSeconds)
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_beginTime).count();
	if (d == Duration::Seconds)
		return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - m_beginTime).count();

	return 0;
}

long long TimeManager::getCurrentTime(Duration d /*= Duration::MicroSeconds*/) const
{
	const auto now = std::chrono::system_clock::now().time_since_epoch();
	if (d == Duration::NanoSeconds)
		return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
	if (d == Duration::MicroSeconds)
		return std::chrono::duration_cast<std::chrono::microseconds>(now).count();
	if (d == Duration::MilliSeconds)
		return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
	if (d == Duration::Seconds)
		return std::chrono::duration_cast<std::chrono::seconds>(now).count();

	return 0;
}
