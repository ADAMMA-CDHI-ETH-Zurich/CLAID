
#pragma once

#include <string>
#include <vector>

namespace claid
{
    class FileUtils
    {
        public:

            static bool deleteDirectory(std::string path);
            static bool dirExists(std::string path);
            static bool fileExists(const std::string& path);

            static bool createDirectoriesRecursively(std::string path);
            static bool createDirectory(std::string path);
            static bool removeDirectoryRecursively(const std::string& path);

            static bool getAllDirectoriesInDirectory(std::string path, std::vector<std::string>& output);
            static bool getAllFilesInDirectory(std::string path, std::vector<std::string>& output);
            static bool getAllFilesInDirectoryRecursively(std::string path, std::vector<std::string>& output);


            static bool readFileToString(std::string path, std::string& content);

            static bool removeFile(const std::string& path);
            static bool removeFileIfExists(const std::string& path);

            static bool copyFileTo(const std::string& source, const std::string& destination, bool appendExistingFile = false);
            static bool moveFileTo(const std::string& source, const std::string& destination, bool appendExistingFile = false);
            

    };
}
