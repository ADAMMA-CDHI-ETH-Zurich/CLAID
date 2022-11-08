#include "Utilities/FileUtils.hpp"

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>

#include <fstream>
#include <sstream>
#include <streambuf>


#include <assert.h>  

#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#else
#include <dirent.h>

// <sys/types.h> and <sys/stat.h> needed, but also needed
// for some of the windows functions. So no additional linux includes.
#endif

namespace claid
{
	bool FileUtils::createDirectory(std::string path)
	{
		// Platform dependent code

		#ifdef _WIN32
		return CreateDirectory(path.c_str(), NULL);
		#else
		return mkdir(path.c_str(), 0744) == -1;
		#endif
	}


	bool FileUtils::dirExists(std::string path)
	{
		struct stat info;

		if (stat(path.c_str(), &info) != 0)
			return false;
		else if (info.st_mode & S_IFDIR)
			return true;
		else
			return false;
	}

	bool FileUtils::fileExists(std::string name)
	{
		struct stat buffer;
		return (stat(name.c_str(), &buffer) == 0);
	}

	bool FileUtils::createDirectoriesRecursively(std::string path)
	{
		std::string delimiter = "\\";

		int pos = path.find(delimiter);

		if (pos == -1)
		{
			delimiter = "/";
			pos = path.find(delimiter);
			if (pos == -1)
			{
				return false;
			}
		}

		std::string subPath;
		
		bool atLeastOneDirectoryCreatedSuccessfully = false;
		while (pos != -1)
		{
			pos = path.find(delimiter, pos + 1);
			subPath = path.substr(0, pos);
			if (FileUtils::createDirectory(subPath))
			{
				atLeastOneDirectoryCreatedSuccessfully = true;
			}
		}
		return atLeastOneDirectoryCreatedSuccessfully;
	}

	bool FileUtils::getAllDirectoriesInDirectory(std::string path, std::vector<std::string>& output)
	{
		#ifdef _WIN32
			// TODO: IMPLEMENT THIS!
			return false;

		#else
			output.clear();
			DIR *dir;
			struct dirent *ent;
			if ((dir = opendir (path.c_str())) != NULL) 
			{
				while ((ent = readdir (dir)) != NULL) 
				{
					std::string directory = std::string(ent->d_name);

					if(directory == "." || directory == "..")
						continue;

					if(FileUtils::dirExists(path + std::string("/") + directory))
					{
						output.push_back(directory);
					}
				}
				closedir (dir);
				return true;
			} 
			else 
			{
				return false;
			}
		#endif
	}
	

	bool FileUtils::readFileToString(std::string path, std::string& content)
	{
		std::ifstream file(path, std::ios::in);
		if(!file.is_open())
		{
			return false;
		}
		content = std::string((std::istreambuf_iterator<char>(file)),
						std::istreambuf_iterator<char>());

		// Remove trailing \n
		content = content.substr(0, content.size() - 1);
		return true;
	}
}