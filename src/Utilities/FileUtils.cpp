#include "Utilities/FileUtils.hpp"

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>

#include <fstream>
#include <sstream>
#include <streambuf>
#include <iostream>

#include <assert.h>  
#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <fileapi.h>
#else
#include <dirent.h>
#include <ftw.h>
int directoryDeleterHelper(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
	return remove(fpath);
}
#endif



namespace claid
{


	#ifdef _WIN32

		bool isDots(const char* str)
		{
			if (strcmp(str, ".") && strcmp(str, ".."))
			{
				return false;
			}
			return true;
		}

		bool FileUtils::removeDirectoryRecursively(const std::string& path)
		{
			HANDLE hFind;  // file handle
			WIN32_FIND_DATAA findFileData;

			std::string dirPath = path + std::string("/*");

			hFind = FindFirstFileA(dirPath.c_str(), &findFileData); // find the first file
			if (hFind == INVALID_HANDLE_VALUE)
				return false;


			bool bSearch = true;
			while (bSearch)
			{
				if (FindNextFileA(hFind, &findFileData))
				{
					if (isDots(findFileData.cFileName))
						continue;

					if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						std::string subDirPath = path + std::string("/") + std::string(findFileData.cFileName);
						// we have found a directory, recurse
						if (!removeDirectoryRecursively(subDirPath))
						{
							FindClose(hFind);
							return false; // directory couldn't be deleted
						}      
					}
					else
					{
						std::string filePath = path + std::string("/") + std::string(findFileData.cFileName);
						// It's a file, delete it.

						if (!DeleteFileA(filePath.c_str()))
						{  // delete the file
							FindClose(hFind);                   
							return false;
						}
					}
				}
				else
				{
					if (GetLastError() == ERROR_NO_MORE_FILES) // no more files there
					{
						bSearch = false;
					}
					else
					{
						// some error occured, close the handle and return FALSE
						FindClose(hFind);
						return false;
					}

				}

			}
			FindClose(hFind);  // closing file handle
			return RemoveDirectoryA(path.c_str()); // remove the empty directory
		}
	#else

	// basically rm -rf
	bool FileUtils::removeDirectoryRecursively(const std::string& path)
	{
		return nftw(path.c_str(), directoryDeleterHelper, 64, FTW_DEPTH | FTW_PHYS) == 0;
	}
	#endif

	bool FileUtils::createDirectory(std::string path)
	{
		// Platform dependent code

		#ifdef _WIN32
		return CreateDirectory(path.c_str(), NULL);
		#else
		std::cout << "Creating " << path << "\n";
		return mkdir(path.c_str(), 0744) == 0;
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

	bool FileUtils::removeFile(const std::string& path)
	{
		return remove(path.c_str()) == 0;
	}

	bool FileUtils::removeFileIfExists(const std::string& path)
	{
		if(!fileExists(path))
		{
			return true;
		}

		return removeFile(path);
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
		pos = 0;
		
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