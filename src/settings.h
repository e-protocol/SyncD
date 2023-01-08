#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include "merge.h"

using namespace std;

class Settings final : public Merge
{
public:
	Settings() {};
	~Settings() {};

	//getters
	string& getBasePath() { return baseDir; }
	string& getTargetPath() { return targetDir; }
	
	//setters
	void setBasePath(string& arg);
	void setTargetPath(string& arg);
	void readSettings();
	void mergeDirs();

private:
	string baseDir = "";
	string targetDir = "";
	const string settingsFolder = "syncDSettings";
	const string settingsFile = "settings.txt";

	void createNewFile();
	void writeSettings();
	void showPath();
};