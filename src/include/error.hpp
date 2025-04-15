#pragma once
#include <unordered_map>
#include <vector>
#include <iostream>
#include <system_error>

struct Error
{
	int count = 1;
	std::vector<std::string> m_messageVec;
	std::string m_type;
	Error() {}
	Error(const std::string& type, const std::string& message) : m_type(type)
	{
		m_messageVec.emplace_back(message);
	}
};

class ErrorMsg
{
	std::unordered_map<std::string, Error> m_errorMsg;
public:
	void add(std::exception& e)
	{
		std::string type;
		const char* ptr = e.what();
		int index = 0;

		while (*ptr != '\0' && *ptr != ':')
		{
			type.push_back(*ptr++);
			index++;
		}

		std::string message;

		while (*ptr != '\0')
			message.push_back(*ptr++);

		auto it = m_errorMsg.find(type);

		if (it != m_errorMsg.end())
		{
			it->second.count++;
			it->second.m_messageVec.emplace_back(message);
			return;
		}

		m_errorMsg[type] = Error(type, message);
	}
	void printErrors()
	{
		for (auto& iter : m_errorMsg)
			std::cout << "Found " << iter.second.count << " errors: " << iter.first << std::endl;
	}
	void printAllErrors()
	{
		if (m_errorMsg.empty())
		{
			std::cout << "No errors occurred" << std::endl;
			return;
		}

		unsigned int index = 1;

		for (auto& it : m_errorMsg)
			for (auto& msg : it.second.m_messageVec)
				std::cout << index++ <<  ") Error " << it.first << msg << std::endl;
	}
	void clear()
	{
		m_errorMsg.clear();
	}
};