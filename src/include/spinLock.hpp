#pragma once
#include <atomic>

class SpinLock
{
	std::atomic_bool m_latch = { false };
public:
	void lock()
	{
		bool ex = false;
		while (!m_latch.compare_exchange_weak(ex, true, std::memory_order_acquire)) { ex = false; }
	}

	void unlock()
	{
		m_latch.store(false, std::memory_order_release);
	}

	bool isLocked()
	{
		return m_latch.load(std::memory_order_acquire);
	}
};