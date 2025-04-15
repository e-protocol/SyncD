#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include "merge.h"

//Singleton
class Settings
{
	std::string m_baseDir;
	std::string m_targetDir;
	const std::string m_settingsDir = "Settings";
	const std::string m_settingsFile = "settings.txt";
	Merge* m_merge;
public:
	Settings() = delete;
	Settings(Merge* m) : m_merge(m) {}
	~Settings() {}
	Settings (const Settings& other) = delete;
	Settings& operator=(const Settings& other) = delete;
	void readSettings();
	void mergeDirs();
	void viewDirStats();

	//setters
	void setPath(const std::string& path, Merge::DirType t);

	//getters
	const std::string& getBasePath() { return m_baseDir; }
	const std::string& getTargetPath() { return m_targetDir; }

private:
	void showPath();
	void createNewFile();
	void writeSettings();
	std::string getCurrentDir();
	bool checkDirs();
};