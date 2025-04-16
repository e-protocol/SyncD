#include "include/merge.h"

Merge::Merge(ErrorMsg* errorMsg) : m_errorMsg(errorMsg)
{
	m_pool = new ThreadPool(THREADS);
}

void Merge::compareDirs()
{
	m_st.setTotalFiles(m_dirTreeBase->getFilesCount() + m_dirTreeTarget->getFilesCount());
	m_taskCount = 2;

	m_pool->addTask([this]()
		{
			m_blockScreen.block("Comparing dirs", &m_st);
		});

	m_pool->addTask([this]() //base to target dirs
		{
			m_dirTreeBase->compareDirs(m_dirTreeBase->getHead(), m_dirTreeTarget->getHead());
			callback(false);
		});

	m_pool->addTask([this]() //target to base dirs
		{
			m_dirTreeTarget->compareDirs(m_dirTreeTarget->getHead(), m_dirTreeBase->getHead());
			callback(false);
		});

	m_pool->wait();
	std::cout << "Non-merged:\tFiles: " << m_dirTreeBase->getUnexistedFiles().size() +
		m_dirTreeTarget->getUnexistedFiles().size() << "\tDirs: "
		<< m_dirTreeBase->getUnexistedDirs().size() +
		m_dirTreeTarget->getUnexistedDirs().size() << std::endl;
}

void Merge::doMerge()
{
	m_taskCount = 1; //avoid blockScreen.wait() called before all tasks are proceed
	m_st.setTotalFiles(m_dirTreeBase->getUnexistedFiles().size() + m_dirTreeTarget->getUnexistedFiles().size() +
		m_dirTreeBase->getUnexistedDirs().size() + m_dirTreeTarget->getUnexistedDirs().size());

	m_pool->addTask([this]()
		{
			m_blockScreen.block("Merging dirs", &m_st);
		});

	m_pool->addTask([this]() 
		{
			removeFiles();
			removeDirs();
			addDirs();
			addFiles();
			callback(false);
		});

	m_pool->wait();
	m_errorMsg->printErrors();
}

void Merge::callback(bool showStats)
{
	m_taskCount--;

	if (m_taskCount == 0)
	{
		m_blockScreen.wait();

		if (showStats)
		{
			if (m_dirTreeBase)
				printStats(BASE);
			if (m_dirTreeTarget)
				printStats(TARGET);
		}
	}
}

void Merge::mergeDirs(const std::string& basePath, const std::string& targetPath)
{
	if (basePath.empty() || targetPath.empty() || basePath == targetPath)
	{
		std::cout << "Invalid dirs!" << std::endl;
		return;
	}

	initDirTree(basePath, targetPath);
	compareDirs();

	if(m_dirTreeBase->getUnexistedDirs().empty() && m_dirTreeBase->getUnexistedFiles().empty() &&
		m_dirTreeTarget->getUnexistedDirs().empty() && m_dirTreeTarget->getUnexistedFiles().empty())
	{
		std::cout << "Target directory is up to date!" << std::endl;
		return;
	}
	doMerge();
	checkRecycleBin();
	freeMemory();
}

void Merge::initDirTree(const std::string& basePath, const std::string& targetPath)
{
	m_errorMsg->clear();
	initDir(basePath, BASE);
	initDir(targetPath, TARGET);

	//call blockScreen.wait() on all tasks proceed
	if (m_dirTreeBase)
		m_taskCount++;
	if (m_dirTreeTarget)
		m_taskCount++;

	m_pool->addTask([this]()
		{
			m_blockScreen.block("Gathering stats");
		});

	if (m_dirTreeBase)
	{
		m_pool->addTask([this]()
			{
				m_dirTreeBase->buildTree(m_dirTreeBase->getHead());
				callback(true);
			});
	}

	if (m_dirTreeTarget)
	{
		m_pool->addTask([this]()
			{
				m_dirTreeTarget->buildTree(m_dirTreeTarget->getHead());
				callback(true);
			});
	}

	m_pool->wait();
	m_errorMsg->printErrors();
}

void Merge::initDir(const std::string& path, const DirType t)
{
	if (path.empty())
		return;

	if (t == BASE && !m_dirTreeBase)
		m_dirTreeBase = new DirTree(path, &m_st);
	else if(t == TARGET && !m_dirTreeTarget)
		m_dirTreeTarget = new DirTree(path, &m_st);
}

void Merge::printStats(const DirType t)
{
	if (t == BASE && m_dirTreeBase)
	{
		std::cout << "Base dir: " << m_dirTreeBase->getHead()->getPath() << "\n";
		std::cout << "Files: " << std::to_string(m_dirTreeBase->getFilesCount());
		std::cout << "\tDirs: " << std::to_string(m_dirTreeBase->getDirsCount()) << "\n";
		std::cout << "Total size: " << fileSizeToString(m_dirTreeBase->getTotalSize()) << std::endl;
	}
	else if(m_dirTreeTarget)
	{
		std::cout << "Target dir: " << m_dirTreeTarget->getHead()->getPath() << "\n";
		std::cout << "Files: " << std::to_string(m_dirTreeTarget->getFilesCount());
		std::cout << "\tDirs: " << std::to_string(m_dirTreeTarget->getDirsCount()) << "\n";
		std::cout << "Total size: " << fileSizeToString(m_dirTreeTarget->getTotalSize()) << std::endl;
	}
}

void Merge::viewDirStats(const std::string& basePath, const std::string& targetPath)
{
	if (basePath.empty() || targetPath.empty() || basePath == targetPath)
	{
		std::cout << "Invalid dirs!" << std::endl;
		return;
	}

	initDirTree(basePath, targetPath);
	freeMemory();
}

void Merge::freeMemory()
{
	if (m_dirTreeBase)
	{
		delete m_dirTreeBase;
		m_dirTreeBase = nullptr;
	}

	if (m_dirTreeTarget)
	{
		delete m_dirTreeTarget;
		m_dirTreeTarget = nullptr;
	}		
}

void Merge::removeDirs() //remove unexisting dirs from target
{
	for (auto& d : m_dirTreeTarget->getUnexistedDirs()) 
	{
		try
		{
			if (exists(d->getPath()))
				remove_all(d->getPath());
		}
		catch (std::exception& e)
		{
			m_errorMsg->add(e);
		}
		m_st.addProgress(1);
	}
}

void Merge::removeFiles() //remove unexisting files from target
{
	for (auto& f : m_dirTreeTarget->getUnexistedFiles())
	{
		try
		{
			if (exists(f))
				remove(f);
		}
		catch (std::exception& e)
		{
			m_errorMsg->add(e);
		}
		m_st.addProgress(1);
	}
}

void Merge::addFiles() //add unexisting files to target
{
	const auto& target = m_dirTreeTarget->getHead()->getPath();
	const auto& base = m_dirTreeBase->getHead()->getPath();

	for (auto& f : m_dirTreeBase->getUnexistedFiles())
	{
		try
		{
			const auto targetParentPath = target / relative(f, base).parent_path();
			copy(f, targetParentPath, std::filesystem::copy_options::overwrite_existing);
		}
		catch (std::exception& e)
		{
			m_errorMsg->add(e);
		}
		m_st.addProgress(1);
	}
}

void Merge::addDirs() //add unexisting dirs to target
{
	std::string target = m_dirTreeTarget->getHead()->getPath().string() + "\\";
	std::string base = m_dirTreeBase->getHead()->getPath().string();

	for (auto& d : m_dirTreeBase->getUnexistedDirs())
	{
		try
		{
			const auto& dir = d->getPath();
			std::filesystem::path p(target + dir.string().substr(base.size(), dir.string().size() - 1));
			std::filesystem::create_directories(p);
		}
		catch (std::exception& e)
		{
			m_errorMsg->add(e);
		}
		m_st.addProgress(1);
	}
}

void Merge::checkRecycleBin()
{
	std::filesystem::path bin(m_dirTreeTarget->getHead()->getPath().string() + "\\$RECYCLE.BIN");

	if (std::filesystem::exists(bin))
		std::filesystem::remove_all(bin);
}