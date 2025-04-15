#pragma once
#include <iostream>
#include <sstream>
#include "settings.h"

using namespace std;
#define CMD_SIZE 9

class Parser
{
	Settings* m_settings;
	ErrorMsg* m_errorMsg;
	const char* cmd[CMD_SIZE] = { "help", "exit", "clear", "base", "target", "merge", "view", "error", "version"};
public:
	Parser(Settings* s, ErrorMsg* errorMsg) : m_settings(s), m_errorMsg(errorMsg) {}

	void parseInput()
	{
		while (true)
		{
			cout << "->";
			vector<string> args;
			readInput(args);
			
			switch (getCmd(args))
			{
				case(0): showHelp(); break;
				case(1): return;
				case(2): system("CLS"); break;
				case(3): m_settings->setPath(getArgument(args, 1), Merge::BASE); break;
				case(4): m_settings->setPath(getArgument(args, 1), Merge::TARGET); break;
				case(5): m_settings->mergeDirs(); break;
				case(6): m_settings->viewDirStats(); break;
				case(7): m_errorMsg->printAllErrors(); break;
				case(8): printVersion(); break;
				default: cout << "Invalid command!\n"; break;
			}
		}
	}

	static void printVersion()
	{
		cout << "********************************************" << endl;
		cout << "syncd - application for merging directories" << endl;
		cout << "created by E-Protocol v1.3" << endl;
		cout << "https://github.com/e-protocol" << endl;
		cout << "********************************************" << endl;
		cout << "type \"help\" to see command list\n" << endl;
	}

private:
	int getCmd(const vector<string>& args)
	{
		if (args.empty())
			return -1;

		for (int i = 0; i < CMD_SIZE; ++i)
			if (args[0] == string(cmd[i]))
				return i;
		return -1;
	}

	void showHelp()
	{
		cout << "Command List:\n";

		for (int i = 0; i < CMD_SIZE; ++i)
		{
			switch (i)
			{
				case(0): cout << cmd[i] << " - view command list" << endl; break;
				case(1): cout << cmd[i] << " - close app" << endl; break;
				case(2): cout << cmd[i] << " - clear console" << endl; break;
				case(3): cout << cmd[i] << " - set base dir, ex: \"base D:\\some_base\\dir\"" << endl; break;
				case(4): cout << cmd[i] << " - set target dir, ex: \"target D:\\some_target\\dir\"" << endl; break;
				case(5): cout << cmd[i] << " - merge dirs" << endl; break;
				case(6): cout << cmd[i] << " - gather dirs data and show stats" << endl; break;
				case(7): cout << cmd[i] << " - print last errors occurred" << endl; break;
				case(8): cout << cmd[i] << " - print app version info" << endl; break;
				default: break;
			}
		}
	}

	const string& getArgument(const vector<string>& args, int i)
	{
		if(args.size() < i)
			return string();
		return args[i];
	}

	void readInput(vector<string>& args)
	{
		string input;
		getline(cin, input);
		stringstream ss(input);

		while (ss >> input)
			args.push_back(input);
	}
};