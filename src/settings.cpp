#include "settings.h"

void Settings::readSettings()
{
	ifstream file;
	file.open(GetCurrentDirectory() + "\\" + settingsFolder + "\\" + settingsFile);

	if (!file.is_open())
		createNewFile();
	else
	{
		vector<string> v;
		string s;

		while (getline(file, s))
		{
			int i = s.find_first_of(' ');

			if (i < 1 || i == s.size() - 1)
			{
				createNewFile();
				return;
			}

			v.push_back(s.substr(0, i - 1)); //get folder type (base_dir or target_dir)
			v.push_back(s.substr(i + 1, s.size() - 1)); //get folder path
		}
			
		file.close();

		if (v.size() != 4)
			createNewFile();
		else
		{
			baseDir = v[1];
			targetDir = v[3];
		}
	}

	showPath();

	if (std::filesystem::is_directory(targetDir) &&
		std::filesystem::is_directory(baseDir))
		Merge::compareDirs(baseDir, targetDir);
}

void Settings::writeSettings()
{
	ifstream file(GetCurrentDirectory() + "\\" + settingsFolder + "\\" + settingsFile);

	!file.is_open() ? createNewFile() : file.close();

	ofstream f(GetCurrentDirectory() + "\\" + settingsFolder + "\\" + settingsFile, ofstream::trunc);

	if (f.is_open())
	{
		f << "base_dir: " << baseDir << "\n"
			<< "target_dir: " << targetDir;
		f.close();
	}
}

void Settings::createNewFile()
{
	if(!std::filesystem::exists(settingsFolder))
		std::filesystem::create_directories(settingsFolder);

	ofstream newFile(GetCurrentDirectory() + "\\" + settingsFolder + "\\" + settingsFile);
	newFile << "base_dir: \ntarget_dir: ";
	newFile.close();
	showPath();
}

void Settings::setBasePath(string& arg)
{
	if (!std::filesystem::is_directory(arg))
	{
		cout << "Invalid directory!" << endl;
		return;
	}

	if (arg == baseDir)
	{
		cout << "target directory should not be equal to base directory!" << endl;
		return;
	}

	baseDir = arg;
	writeSettings();
	fileStats(arg, Merge::BASE);
}

void Settings::setTargetPath(string& arg)
{
	if (!std::filesystem::is_directory(arg))
	{
		cout << "Invalid directory!" << endl;
		return;
	}

	if (arg == baseDir)
	{
		cout << "target directory should not be equal to base directory!" << endl;
		return;
	}

	targetDir = arg;
	writeSettings();
	fileStats(arg, Merge::TARGET);

	if (!std::filesystem::is_directory(baseDir))
		Merge::compareDirs(baseDir, targetDir);
}

void Settings::showPath()
{
	cout << "base Dir: " << baseDir << "\n" <<
		"target Dir: " << targetDir << "\n" << endl;

	if (std::filesystem::is_directory(baseDir))
	{
		fileStats(baseDir, Merge::BASE);
		cout << endl;
	}

	if (std::filesystem::is_directory(targetDir))
	{
		fileStats(targetDir, Merge::TARGET);
		cout << endl;
	}
}

void Settings::mergeDirs()
{
	if (baseDir.empty())
	{
		cout << "Invalid base directory!" << endl;
		return;
	}

	if (targetDir.empty())
	{
		cout << "Invalid target directory!" << endl;
		return;
	}

	Merge::mergeDirs(baseDir, targetDir);
}