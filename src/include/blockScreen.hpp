#pragma once
#include <iostream>
#include <windows.h>
#include "misc.hpp"

class BlockScreen
{
	SpinLock m_lock;
	SpinLock m_printLock;
public:
	void block(std::string&& str, FileStat* st = nullptr)
	{
		m_lock.lock();
		m_printLock.lock();
		setCursorVisibility(false);
		int i = 0;
		std::cout << str;

		while (m_printLock.isLocked())
		{
			if (!st)
			{
				if (i > 2)
				{
					i = 0;
					std::cout << std::flush << "\r" << str;
					continue;
				}

				std::cout << ".";
				i++;
			}
			else
			{
				if (i > 2)
				{
					i = 0;
					std::cout << std::flush << "\r" << str << "   ";
					std::cout << st->getProgress() << "%";
					continue;
				}

				std::cout << std::flush << "\r" << str;
				i++;
				std::string dot;

				for (int k = 0; k < 3; ++k)
				{
					if (k < i)
						dot.push_back('.');
					else
						dot.push_back(' ');
				}
				std::cout << dot << st->getProgress() << "%";
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		std::cout << std::flush << "\r" << str << "...Done!" << std::endl;
		setCursorVisibility(true);
		m_lock.unlock();
	}

	void wait()
	{
		m_printLock.unlock();
		while (m_lock.isLocked()) {}
	}

private:
	void setCursorVisibility(bool status)
	{
		HANDLE handle = NULL;
		CONSOLE_CURSOR_INFO curInfo;
		handle = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleCursorInfo(handle, &curInfo);
		curInfo.bVisible = status;
		SetConsoleCursorInfo(handle, &curInfo);
	}
};