#pragma once
#include <vector>
#include <filesystem>
#include <atomic>
#include <sys/types.h>
#include <sys/stat.h>
#include "misc.hpp"

//no modification during concurrent
class Dir
{
	std::filesystem::path m_path;
	std::vector<Dir*> m_dirs;
	std::vector<std::filesystem::path> m_files;
public:
	Dir() = delete;
	Dir(const std::filesystem::path& p) : m_path(p) {}
	~Dir()
	{
		for (auto& d : m_dirs)
			delete d;
	}

	const std::filesystem::path& getPath()
	{
		return m_path;
	}

	const std::vector<Dir*>& getDirs() { return m_dirs; }
	const std::vector<std::filesystem::path>& getFiles() { return m_files; }

	void addSubDir(Dir* dir)
	{
		m_dirs.push_back(dir);
	}

	void addFile(const std::filesystem::path& p)
	{
		m_files.emplace_back(p);
	}

	void removeSubDir(const std::filesystem::path& p)
	{
		for(size_t i = 0; i < m_dirs.size(); ++i)
			if (m_dirs[i]->getPath() == p)
			{
				delete m_dirs[i];
				m_dirs.erase(m_dirs.begin() + i);
				break;
			}
	}

	void removeFile(const std::filesystem::path& p)
	{
		for (size_t i = 0; i < m_files.size(); ++i)
			if (m_files[i] == p)
			{
				m_dirs.erase(m_dirs.begin() + i);
				break;
			}
	}
};

//no modification during concurrent
class DirTree
{
	Dir* m_head = nullptr;
	size_t m_filesCount = 0;
	size_t m_dirsCount = 0;
	size_t m_totalSize = 0;
	std::vector<Dir*> m_unexistedDirs; //store copy of pointers to Dir
	std::vector<std::filesystem::path> m_unexistedFiles;
	FileStat* m_st = nullptr;
public:
	DirTree() = delete;
	DirTree(const std::filesystem::path& path, FileStat* stat) : m_st(stat)
	{
		m_head = new Dir(path);
	}
	~DirTree()
	{
		delete m_head;
	}

	Dir* getHead() { return m_head; }
	size_t getTotalSize() { return m_totalSize; }
	size_t getFilesCount() { return m_filesCount; }
	size_t getDirsCount() { return m_dirsCount; }
	const std::vector<Dir*>& getUnexistedDirs() { return m_unexistedDirs; }
	const std::vector<std::filesystem::path>& getUnexistedFiles() { return m_unexistedFiles; }

	void buildTree(Dir* upDir) //recursive func
	{
		using namespace std::filesystem;

		for (const auto& obj : directory_iterator(upDir->getPath(), directory_options::skip_permission_denied))
		{
			const auto& path = obj.path();

			if (checkSysHiddenFile(path.string()))
				continue;

			if (is_directory(path))
			{
				Dir* dir = new Dir(path);
				upDir->addSubDir(dir);
				m_dirsCount++;
				buildTree(dir);
			}
			else
			{
				m_totalSize += file_size(path);
				m_filesCount++;
				upDir->addFile(path);
			}
		}
	}

	void doCompareDirs(std::vector<std::pair<Dir*, Dir*>>& pairDirs, const std::vector<Dir*>& dirs,
						const std::vector<Dir*>& otherDirs, bool isDir)
	{
		for (auto& d : dirs)
		{
			bool found = false;

			for (auto& o : otherDirs)
				if (d->getPath().filename() == o->getPath().filename())
				{
					pairDirs.emplace_back(std::pair(d, o));
					found = true;
					break;
				}

			if (!found)
			{
				pairDirs.emplace_back(std::pair(d, nullptr));

				if(isDir)
					m_unexistedDirs.emplace_back(d);
			}
		}
	}

	void compareDirs(Dir* upDir, Dir* otherUpDir) //recursive func
	{
		using namespace std::filesystem;
		const std::vector<Dir*>& dirs = upDir->getDirs();

		if (!otherUpDir) //if not existed, all elements are unexisted
		{
			for (auto& d : dirs)
				m_unexistedDirs.emplace_back(d);

			compareFiles(upDir, otherUpDir);

			//repeat for all internal dirs
			for (auto& d : dirs)
				compareDirs(d, nullptr);
			return;
		}

		compareFiles(upDir, otherUpDir);
		const std::vector<Dir*>& otherDirs = otherUpDir->getDirs();
		std::vector<std::pair<Dir*, Dir*>> pairDirs; //create same dir pairs on same layer
		pairDirs.reserve(dirs.size() + otherDirs.size()); //avoid realloc
		doCompareDirs(pairDirs, dirs, otherDirs, true);

		//repeat for all internal dirs
		for (auto& p : pairDirs)
			compareDirs(p.first, p.second);
	}

	void compareFiles(Dir* dir, Dir* otherDir)
	{
		using namespace std::filesystem;
		const auto& files = dir->getFiles();
		const auto& otherFiles = otherDir->getFiles();

		if (!otherDir) //if not existed, all elements are unexisted
		{
			for (auto& f : files)
				m_unexistedFiles.emplace_back(f);

			m_st->addProgress(files.size());
			return;
		}
		
		for (auto& f : files)
		{
			bool found = false;

			for (auto& o : otherFiles)
			{
				if (f.filename() == o.filename() &&
					last_write_time(f) == last_write_time(o))
				{
					found = true;
					break;
				}
			}

			if (!found)
				m_unexistedFiles.emplace_back(f);
		}

		m_st->addProgress(files.size());
	}
};