#pragma once
#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>
#include <future>
#include "vectorM.h"
#include "static.h"

class Merge
{
public:
	Merge() {};
	~Merge() {};

	enum DirType
	{
		BASE,
		TARGET
	};

	void fileStats(std::string& path, const DirType t);
	void compareDirs(std::string& base, std::string &target);
	void mergeDirs(std::string& base, std::string &target);

private:
	std::vector<std::filesystem::path> baseDirFiles;
	std::vector<std::filesystem::path> targetDirFiles;
	std::vector<std::filesystem::path> baseDirs;
	std::vector<std::filesystem::path> targetDirs;
	Vector<std::filesystem::path> filesToRemove;
	Vector<std::filesystem::path> filesToAdd;
	Vector<std::filesystem::path> dirsToRemove;
	Vector<std::filesystem::path> dirsToAdd;
	bool asyncStatus = false;
	std::future<void> asyncOp;

	std::string sizeString(double size);
	std::map<std::string, std::pair<size_t, size_t>> extStats(const std::filesystem::path& dir, const DirType t);
	template<class Iter> Iter findPath(const Iter &begin, const Iter &end, 
		std::filesystem::path &p, const int baseSize, const int targetSize);
	void showProgress(int index, const int maxCount);
	void showGatherInfo(const std::string &str);
	void baseToTargetFilesComparsion(std::filesystem::path &p, std::string& base, std::string& target);
	void targetToBaseFilesComparsion(std::filesystem::path& p, std::string& base, std::string& target);
	void baseToTargetDirsComparsion(std::filesystem::path& b, std::string& base, std::string& target);
	void targetToBaseDirsComparsion(std::filesystem::path& t, std::string& base, std::string& target);
};