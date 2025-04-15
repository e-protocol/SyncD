#pragma once
#include <windows.h>
#include <string>
#include "spinLock.hpp"

static bool checkSysHiddenFile(const std::string& p)
{
	DWORD attr = GetFileAttributes(p.c_str());

	if (attr & (FILE_ATTRIBUTE_HIDDEN || FILE_ATTRIBUTE_SYSTEM))
		return true;
	return false;
}

static std::string fileSizeToString(size_t size)
{
	int i = 0;
	const size_t d = 1024.0;
	size_t tail = 0;

	while(size / d > 0)
	{
		tail = size % d;
		size /= d;
		i++;
	}

	while (tail > 99) { tail /= 10; }
	std::string tailStr = tail == 0 ? "" : "." + std::to_string(tail);
	std::string bytename;

	switch (i)
	{
		case(0): bytename = "b"; break;
		case(1): bytename = "Kb"; break;
		case(2): bytename = "Mb"; break;
		case(3): bytename = "Gb"; break;
		case(4): bytename = "Tb"; break;
		default: bytename = "Pb"; break;
	}

	return std::to_string(size) + tailStr + " " + bytename;
}

class FileStat
{
	size_t m_totalFiles = 0;
	size_t m_progress = 0;
	SpinLock m_lock;
public:
	FileStat() {}
	FileStat(size_t size) : m_totalFiles(size) {}

	void setTotalFiles(size_t s) 
	{
		m_lock.lock();
		m_totalFiles = s;
		m_progress = 0;
		m_lock.unlock();
	}

	void addProgress(size_t processedFiles)
	{
		m_lock.lock();
		m_progress += processedFiles;
		m_lock.unlock();
	}

	size_t getProgress()
	{
		m_lock.lock();
		size_t s = m_progress * 100 / m_totalFiles;
		m_lock.unlock();
		return s;
	}
};