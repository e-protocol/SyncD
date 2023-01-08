#pragma once
#include <vector>
#include <mutex>

template<typename T>
class Vector
{
	std::vector<T> v;
	std::mutex mutex;

public:
	void push_back(T t)
	{
		mutex.lock();
		v.push_back(t);
		mutex.unlock();
	}
	void push_front(T t)
	{
		mutex.lock();
		v.push_front(t);
		mutex.unlock();
	}
	void pop_back()
	{
		mutex.lock();
		v.pop_back();
		mutex.unlock();
	}
	void pop_front()
	{
		mutex.lock();
		v.pop_front();
		mutex.unlock();
	}
	const int size() { return v.size(); }
	bool empty() { return v.empty(); }
	const auto begin() { return v.begin(); }
	const auto end() { return v.end(); }
	void clear() { v.clear(); }
	const auto rbegin() { return v.rbegin(); }
	const auto rend() { return v.rend(); }
};