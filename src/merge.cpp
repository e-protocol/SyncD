#include "merge.h"
#include <thread>
#include <sys/types.h>
#include <sys/stat.h>
#include <codecvt>
#ifndef WIN32
#include <unistd.h>
#endif

using namespace std::filesystem;
using namespace std;

string Merge::sizeString(double size)
{
	int i = 3;

	for (; i > 0; i--)
	{
		if (size < 1024.0)
			break;

		size /= 1024.0;
	}

	string str;
	size = size_t(size * 100.0) / 100.0;

	switch (i)
	{
		case(0): str = "Gb"; break;
		case(1): str = "Mb"; break;
		case(2): str = "Kb"; break;
		case(3): str = "b"; break;
		default: break;
	}

	return (stringstream() << size << " " << str).str();
}

map<string, pair<size_t, size_t>> Merge::extStats(const path& dir, const DirType t)
{
	map <string, pair<size_t, size_t>> m;

	for (const auto& entry : recursive_directory_iterator(dir, directory_options::skip_permission_denied))
	{
		const path path(entry.path().string());
		const string ext(path.extension().string());

		//if (ext.length() == 0)
			//continue;

		if (checkSysHiddenFile(path))
			continue;

		const size_t size(file_size(path));
		auto& [size_accum, count] = m[ext];
		size_accum += size;
		count++;

		if (is_directory(path))
		{
			t == BASE ? baseDirs.push_back(path)
				: targetDirs.push_back(path);
			continue;
		}
		else
			t == BASE ? baseDirFiles.push_back(path) 
					  : targetDirFiles.push_back(path);
	}

	return m;
}

void Merge::showGatherInfo(const std::string &str)
{
	int i = 0;
	setCursorVisibility(false);

	while (asyncStatus)
	{
		cout << "\r" << str;

		if (i > 3)
		{
			i = 0;

			for (int k = 0; k < 20; k++)
				cout << " ";
			cout << "\r";
		}

		if(i > 0)
			for (int k = 1; k < i + 1; k++)
				cout << ".";

		i++;
		Sleep(500);
	}

	cout << "\r" << str << "...Done!" << endl;
	setCursorVisibility(true);
}

void Merge::fileStats(string &path, const DirType t)
{
	asyncStatus = true;
	asyncOp = async(launch::async, &Merge::showGatherInfo, this, "Gathering stats");
	t == BASE ? baseDirFiles.clear(), baseDirs.clear()
		: targetDirFiles.clear(), targetDirs.clear();

	double totalSize = 0.0;
	size_t totalCount = 0;

	try 
	{
		for (const auto& [ext, stats] : extStats(path, t))
		{
			const auto& [accumSize, count] = stats;
			totalSize += accumSize;
			totalCount += count;
		}
	}
	catch (exception& e)
	{
		asyncStatus = false;
		asyncOp.wait();
		cout << "Error: " << e.what() << endl;
		return;
	}

	asyncStatus = false;
	asyncOp.wait();
	cout << (t == BASE ? "base Dir stats:\n" : "target Dir stats:\n") << 
		 "Total Objects: " << totalCount << "\n"
		 "Total Size: " << sizeString(totalSize) << endl;
}

//compare files by their location path excluding baseDir and targetDir paths
template<class Iter>
Iter Merge::findPath(const Iter &begin, const Iter &end, std::filesystem::path &p, 
					 const int baseSize, const int targetSize)
{
	for (Iter it = begin; it != end; it++)
	{
		std::filesystem::path fp = *it;
		string fpStr = fp.string().substr(targetSize, fp.string().size() - 1);
		string pStr = p.string().substr(baseSize, p.string().size() - 1);

		if(pStr == fpStr)
			return it;
	}
	return end;
}

void Merge::baseToTargetFilesComparsion(std::filesystem::path& p, std::string& base, std::string& target)
{
	auto it = findPath(begin(targetDirFiles), end(targetDirFiles), p, base.size(), target.size());

	if (it == targetDirFiles.end())
		filesToAdd.push_back(p);
	else
	{
		struct stat result;
		stat(p.string().c_str(), &result);
		auto baseTime = result.st_mtime;
		path t = *it;
		stat(t.string().c_str(), &result);
		auto targetTime = result.st_mtime;

		if (baseTime > targetTime)
		{
			filesToRemove.push_back(t);
			filesToAdd.push_back(p);
		}
	}
}

void Merge::targetToBaseFilesComparsion(std::filesystem::path &p, std::string &base, std::string &target)

{
	auto it = findPath(begin(baseDirFiles), end(baseDirFiles), p, target.size(), base.size());

	if (it == baseDirFiles.end())
		filesToRemove.push_back(p);
}

void Merge::baseToTargetDirsComparsion(std::filesystem::path& b, std::string& base, std::string& target)
{
	auto it = findPath(begin(targetDirs), end(targetDirs), b, base.size(), target.size());

	if (it == targetDirs.end())
		dirsToAdd.push_back(b);
}

void Merge::targetToBaseDirsComparsion(std::filesystem::path& t, std::string& base, std::string& target)
{
	auto it = findPath(begin(baseDirs), end(baseDirs), t, target.size(), base.size());

	if (it == baseDirs.end())
		dirsToRemove.push_back(t);
}

void Merge::compareDirs(std::string &base, std::string &target)
{
	if (baseDirFiles.empty())
		return;

	asyncStatus = true;
	asyncOp = async(launch::async, &Merge::showGatherInfo, this, "Comparing dirs");
	filesToAdd.clear();
	filesToRemove.clear();
	dirsToRemove.clear();
	dirsToAdd.clear();
	vector<future<void>> asyncV;
	int threadNum = thread::hardware_concurrency() / 2;

	if (threadNum < 1)
		threadNum = 2;

	for (int j = 0; j < threadNum; j++)
		asyncV.push_back(future<void>());

	//files comparsion
	for (int k = 0; k < baseDirFiles.size(); k++)
	{
		for (int j = 0; j < threadNum; j++)
			if (k < baseDirFiles.size())
			{
				asyncV[j] = async(launch::async, &Merge::baseToTargetFilesComparsion, this, baseDirFiles[k], base, target);

				if(j < threadNum - 1)
					k++;
			}

		for (int j = threadNum - 1; j > -1; j--)
			asyncV[j].wait();
	}

	for (int k = 0; k < targetDirFiles.size(); k++)
	{
		for (int j = 0; j < threadNum; j++)
			if (k < targetDirFiles.size())
			{
				asyncV[j] = async(launch::async, &Merge::targetToBaseFilesComparsion, this, targetDirFiles[k], base, target);
				
				if (j < threadNum - 1)
					k++;
			}

		for (int j = threadNum - 1; j > -1; j--)
			asyncV[j].wait();
	}

	//dirs comparsion
	for (int k = 0; k < baseDirs.size(); k++)
	{
		for (int j = 0; j < threadNum; j++)
			if (k < baseDirs.size())
			{
				asyncV[j] = async(launch::async, &Merge::baseToTargetDirsComparsion, this, baseDirs[k], base, target);

				if (j < threadNum - 1)
					k++;
			}

		for (int j = threadNum - 1; j > -1; j--)
			asyncV[j].wait();
	}

	for (int k = 0; k < targetDirs.size(); k++)
	{
		for (int j = 0; j < threadNum; j++)
			if (k < targetDirs.size())
			{
				asyncV[j] = async(launch::async, &Merge::targetToBaseDirsComparsion, this, targetDirs[k], base, target);

				if (j < threadNum - 1)
					k++;
			}

		for (int j = threadNum - 1; j > -1; j--)
			asyncV[j].wait();
	}

	asyncStatus = false;
	asyncOp.wait();
	cout << "Compare stats:\n"
		<< "Objects to add: " << filesToAdd.size() + dirsToAdd.size() << "\n"
		<< "Objects to remove: " << filesToRemove.size() + dirsToRemove.size() << endl;
}

void Merge::mergeDirs(std::string &base, std::string &target)
{
	//update directories stats
	baseDirFiles.clear();
	targetDirFiles.clear();
	baseDirs.clear();
	targetDirs.clear();
	asyncStatus = true;
	const std::string info = "";
	asyncOp = async(launch::async, &Merge::showGatherInfo, this, "Gathering stats");
	try
	{
		for (const auto& [ext, stats] : extStats(base, BASE)) {};
		for (const auto& [ext, stats] : extStats(target, TARGET)) {};
	}
	catch (exception& e)
	{
		asyncStatus = false;
		asyncOp.wait();
		cout << "Error: " << e.what() << endl;
		return;
	}

	asyncStatus = false;
	asyncOp.wait();

	if (baseDirFiles.empty())
	{
		cout << "Base directory is empty!" << endl;
		return;
	}

	compareDirs(base, target); //find changes in directories

	if (filesToAdd.empty() && filesToRemove.empty() && 
		dirsToRemove.empty() && dirsToAdd.empty())
	{
		cout << "Target directory is up to date!" << endl;
		return;
	}

	int i = 1;
	int maxCount = filesToAdd.size() + filesToRemove.size() + dirsToRemove.size() + dirsToAdd.size();
	setCursorVisibility(false); //avoid blinking cursor on progress bar update

	try
	{
		for (auto& r : filesToRemove)
		{
			remove(r);
			showProgress(i++, maxCount);
		}

		for (auto it = dirsToRemove.rbegin(); it != dirsToRemove.rend(); it++)
		{
			if (exists(*it))
				remove_all(*it);

			showProgress(i++, maxCount);
		}

		const auto copyOptions = copy_options::update_existing
			| copy_options::recursive;

		for (auto it = dirsToAdd.rbegin(); it != dirsToAdd.rend(); it++)
		{
			path d(*it);
			path p(target + "\\" + d.string().substr(base.size(), d.string().size() - 1));
			create_directories(p);
			showProgress(i++, maxCount);
		}

		//recursively copy directories if not exists
		for (auto& a : filesToAdd)
		{
			// Create path in target, if not existing.
			const auto targetParentPath = path(target) / relative(a, base).parent_path();

			// Copy to the targetParentPath which we just created.
			copy(a, targetParentPath, copy_options::overwrite_existing);
			showProgress(i++, maxCount);
		}
	}
	catch (exception &e)
	{
		cout << "Error: " << e.what() << endl;
		return;
	}

	cout << "\nMerge Complete!" << endl;
	setCursorVisibility(true); //set cursor visible again
}

void Merge::showProgress(int index, const int maxCount)
{
	cout << "\rMerging: ";

	for (int k = 0; k <= 50; k++)
		getMapVal(index, 0, maxCount, 0, 50) >= k ? cout << "#" : cout << ".";

	cout << " " << (int)((double)index / (double)maxCount * 100.0) << "%" << flush;
}