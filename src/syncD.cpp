#include "syncD.h"
#include "settings.h"
#include "static.h"

using namespace std;

vector<string> commandList{ "help", "base", "target", "merge", "exit" };
Settings settings;

void showHelp()
{
	cout << "Command Lits:\n" <<
		commandList[0] << " - show all commands\n" <<
		commandList[1] << " - path to base directory. Example: base D:\\BaseDir\\dir\n" <<
		commandList[2] << " - path to target directory. Example: base D:\\TargetDir\\dir\n" <<
		commandList[3] << " - merge directories\n" <<
		commandList[4] << " - exit application" << endl;
}

bool checkInput()
{
	cout << "->";
	string input;
	getline(cin, input);
	string arg = getCommandArgument(input);

	switch (getCommand(commandList, input))
	{
		case(0): showHelp(); break;
		case(1): settings.setBasePath(arg); break;
		case(2): settings.setTargetPath(arg); break;
		case(3): settings.mergeDirs(); break;
		case(4): return false;
		default: cout << "Invalid command!" << endl; break;
	}

	return true;
}

int main()
{
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	setlocale(0, "");
	cout << "********************************************" << endl;
	cout << "syncD - application for merging directories" << endl;
	cout << "created by E-Protocol v1.0" << endl;
	cout << "https://github.com/e-protocol" << endl;
	cout << "********************************************" << endl;
	cout << "type \"help\" to see commands list" << endl;
	settings.readSettings();

	while (checkInput()) {};
	return 0;
}
