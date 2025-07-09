#pragma once

#include <vector>
#include <functional>
#include <memory>

using Coroutine = std::function<bool(float)>;

class CoroutineSystem
{
public:
	CoroutineSystem() = default;
	~CoroutineSystem() = default;

	void addCoroutine(const Coroutine& coroutine);
	//void removeCoroutine(coroutine coroutine);
	void removeCoroutine(int index);

private:
	friend class Scene;
	std::vector<std::shared_ptr<Coroutine>> getAllCoroutines() const;
	void clear();

private:
	std::vector<std::shared_ptr<Coroutine>> m_coroutines;
};
