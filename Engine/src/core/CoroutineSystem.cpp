#include "CoroutineSystem.h"
#include "Logger.h"

void CoroutineSystem::addCoroutine(const Coroutine& coroutine)
{
	auto ptr = std::make_shared<Coroutine>(coroutine);
	m_coroutines.emplace_back(ptr);
}

//void CoroutineSystem::removeCoroutine(coroutine coroutine)
//{
//	auto iter = std::find(m_coroutines.begin(), m_coroutines.end(), coroutine);
//
//	if (iter != m_coroutines.end())
//		m_coroutines.erase(iter);
//}

void CoroutineSystem::removeCoroutine(int index)
{
	if (index > m_coroutines.size())
	{
		logError("Index out of bounds specified: " + index);
		return;
	}

	m_coroutines.erase(m_coroutines.begin() + index);
}

std::vector<std::shared_ptr<Coroutine>> CoroutineSystem::getAllCoroutines() const
{
	return m_coroutines;
}

void CoroutineSystem::clear()
{
	m_coroutines.clear();
}
