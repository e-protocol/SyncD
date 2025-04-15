#pragma once
#include <iostream>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadPool
{
	std::queue<std::function<void()>> m_tasks; //store tasks
	std::vector<std::thread> m_pool;
	std::mutex m_mutex;
	std::condition_variable m_cv;
	std::atomic<bool> m_quit = false;
	std::atomic<size_t> m_process = 0;
public:
	ThreadPool(size_t num)
	{
		m_pool.reserve(num);

		for (size_t k = 0; k < num; ++k)
			m_pool.emplace_back(&ThreadPool::run, this, k);
	}
	~ThreadPool()
	{
		m_quit = true;
		m_cv.notify_all();
		clear();

		for (size_t k = 0; k < m_pool.size(); ++k)
			m_pool[k].join();

		m_pool.clear();
	}

	ThreadPool(const ThreadPool& other) = delete;
	ThreadPool& operator=(const ThreadPool& other) = delete;
	ThreadPool(ThreadPool&& other) = delete;
	ThreadPool& operator=(ThreadPool&& other) = delete;

	void addTask(const auto& func)
	{
		if (m_quit)
			return;

		m_process++;
		{
			std::lock_guard lock(m_mutex);
			m_tasks.push(func);
		}
		m_cv.notify_one();
	}

	void clear()
	{
		std::lock_guard lock(m_mutex);
		m_tasks = {};
	}

	void wait()
	{
		while (m_process) {}
	}
private:
	void run(size_t i)
	{
		while (!m_quit)
		{
			std::function<void()> task;

			{
				std::unique_lock lock(m_mutex);
				m_cv.wait(lock, [this]() { return !m_tasks.empty() || m_quit; });

				if (m_quit)
					return;

				task = std::move(m_tasks.front());
				m_tasks.pop();
			}

			task();
			m_process--;
		}
	}
};