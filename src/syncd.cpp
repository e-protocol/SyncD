#include "include/parser.hpp"

using namespace std;

int main()
{
#ifdef _WIN32
	{}
#else
	cout << "this app designed for Windows only\n";
	exit(0);
#endif

	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);
	Parser::printVersion();
	ErrorMsg em;
	Merge m(&em);
	Settings s(&m);
	s.readSettings();
	Parser p(&s, &em);
	p.parseInput();
	return 0;
}
