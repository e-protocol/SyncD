#pragma once
#include <filesystem>
#include <windows.h>
#include <string>
#include <vector>

static int getMapVal(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static void setCursorVisibility(bool status)
{
	HANDLE hStdOut = NULL;
	CONSOLE_CURSOR_INFO curInfo;
	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleCursorInfo(hStdOut, &curInfo);
	curInfo.bVisible = status;
	SetConsoleCursorInfo(hStdOut, &curInfo);
}

static bool checkSysHiddenFile(const std::filesystem::path& p)
{
	DWORD attr = GetFileAttributes(p.string().c_str());

	if (attr & (FILE_ATTRIBUTE_HIDDEN || FILE_ATTRIBUTE_SYSTEM))
		return true;
	return false;
}

static std::string GetCurrentDirectory()
{
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");

	return std::string(buffer).substr(0, pos);
}

static int getCommand(std::vector<std::string> &v, std::string& s)
{
	std::string command;

	for (auto& c : s)
	{
		if (c == ' ' || c == '\n')
			break;

		command.push_back(c);
	}

	for (int k = 0; k < v.size(); k++)
		if (v[k] == command)
			return k;

	return -1;
}

static std::string getCommandArgument(std::string& c)
{
	if (c.empty())
		return "";

	return c.substr(c.find(' ') + 1, c.size());
}