#pragma once

#include <deque>
#include <functional>
#include <future>
#include <mutex>

#include <nlohmann/json.hpp>

// Runs jobs posted from other threads (the MCP server) on the editor main thread,
// the only thread allowed to touch GL, the ECS registry and ImGui
class MainThreadQueue
{
public:
	using Result = nlohmann::json;

	// Exceptions thrown by the job come back through future.get().
	// After close() the returned future fails with broken_promise.
	std::future<Result> post(std::function<Result()> job)
	{
		std::packaged_task<Result()> task(std::move(job));
		auto future = task.get_future();

		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_closed)
				m_jobs.push_back(std::move(task));
		}

		return future;
	}

	// Called once per frame from EditorApp::update()
	void drain()
	{
		std::deque<std::packaged_task<Result()>> jobs;
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			jobs.swap(m_jobs);
		}

		for (auto& job : jobs)
			job();
	}

	// Drops pending jobs so threads waiting on them are released during shutdown
	void close()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_closed = true;
		m_jobs.clear();
	}

private:
	std::mutex m_mutex;
	std::deque<std::packaged_task<Result()>> m_jobs;
	bool m_closed = false;
};
