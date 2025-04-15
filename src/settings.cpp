#include "include/settings.h"
#include <windows.h>
#include <codecvt>

void Settings::readSettings()
{
	std::ifstream file;
	file.open(getCurrentDir() + "\\" + m_settingsDir + "\\" + m_settingsFile);

	if (!file.is_open())
		createNewFile();
	else
	{
		std::string s;

		while (std::getline(file, s))
		{
			std::string str = s.substr(s.find(' ') + 1, s.size());
			
			if (str.empty())
				continue;

			if (m_baseDir.empty())
				m_baseDir = str;
			else
				m_targetDir = str;
		}

		file.close();
	}

	if (!checkDirs())
	{
		showPath();
		return;
	}

	if (std::filesystem::is_directory(m_baseDir))
		m_merge->initDir(std::move(m_baseDir), Merge::BASE);

	if (std::filesystem::is_directory(m_targetDir))
		m_merge->initDir(std::move(m_targetDir), Merge::TARGET);

	showPath();
	m_merge->initDirTree(m_baseDir, m_targetDir);
}

bool Settings::checkDirs()
{
	bool ok = true;

	if (!m_baseDir.empty() && !std::filesystem::exists(m_baseDir))
	{
		std::cout << "Error: Not exist base_dir: " << m_baseDir << std::endl;
		ok = false;
	}

	if (!m_targetDir.empty() && !std::filesystem::exists(m_targetDir))
	{
		std::cout << "Error: Not exist target_dir: " << m_targetDir << std::endl;
		ok = false;
	}

	if (m_baseDir.empty() && m_targetDir.empty())
		return false;

	return ok;
}

void Settings::showPath()
{
	std::cout << "base_dir: " << m_baseDir << "\n"
		<< "target_dir: " << m_targetDir << std::endl;
}

void Settings::writeSettings()
{
	std::ifstream file(getCurrentDir() + "\\" + m_settingsDir + "\\" + m_settingsFile);

	!file.is_open() ? createNewFile() : file.close();

	std::ofstream f(getCurrentDir() + "\\" + m_settingsDir + "\\" + m_settingsFile, std::ofstream::trunc);

	if (f.is_open())
	{
		f << "base_dir: " << m_baseDir << "\n"
			<< "target_dir: " << m_targetDir;
		f.close();
	}
}

void Settings::createNewFile()
{
	if (!std::filesystem::exists(m_settingsDir))
		std::filesystem::create_directories(m_settingsDir);

	std::ofstream newFile(getCurrentDir() + "\\" + m_settingsDir + "\\" + m_settingsFile);
	newFile << "base_dir: \ntarget_dir: ";
	newFile.close();
}

void Settings::setPath(const std::string& path, Merge::DirType t)
{
	if (!std::filesystem::is_directory(path))
	{
		std::string str = t == Merge::BASE ? "base" : "target";
		std::cout << "Invalid " << str <<  " directory!" << std::endl;
		return;
	}

	if(t == Merge::BASE)
		m_baseDir = path;
	else
		m_targetDir = path;

	writeSettings();
	m_merge->initDir(path, t);
}

void Settings::mergeDirs()
{
	if (m_baseDir.empty() || m_targetDir.empty() || m_baseDir == m_targetDir)
	{
		std::cout << "Invalid dirs!" << std::endl;
		return;
	}

	if (checkDirs())
		m_merge->mergeDirs(m_baseDir, m_targetDir);
}

void Settings::viewDirStats()
{
	if (m_baseDir.empty() && m_targetDir.empty())
	{
		showPath();
		return;
	}

	if(checkDirs())
		m_merge->initDirTree(m_baseDir, m_targetDir);
}

std::string Settings::getCurrentDir()
{
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\");
	return std::string(buffer).substr(0, pos);
}