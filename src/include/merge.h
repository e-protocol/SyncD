#pragma once
#include <unordered_map>
#include "threadPool.hpp"
#include "dirTree.hpp"
#include "blockScreen.hpp"
#include "error.hpp"
#define THREADS 3

class Merge
{
	ThreadPool* m_pool;
	DirTree* m_dirTreeBase = nullptr;
	DirTree* m_dirTreeTarget = nullptr;
	std::atomic<int> m_taskCount = 0;
	BlockScreen m_blockScreen;
	FileStat m_st;
	ErrorMsg* m_errorMsg;
	
public:
	Merge(ErrorMsg* errorMsg);
	~Merge()
	{
		delete m_pool;
		delete m_dirTreeBase;
		delete m_dirTreeTarget;
	}
	enum DirType
	{
		BASE,
		TARGET
	};

	void mergeDirs(const std::string& basePath, const std::string& targetPath);
	void viewDirStats(const std::string& basePath, const std::string& targetPath);

private:
	void printStats(const DirType t);
	void compareDirs();
	void doMerge();
	void initDir(const std::string& path, const DirType t);
	void initDirTree(const std::string& basePath, const std::string& targetPath);
	void callback(bool showStats);
	void freeMemory();
	void removeFiles();
	void removeDirs();
	void addFiles();
	void addDirs();
	void checkRecycleBin();
};