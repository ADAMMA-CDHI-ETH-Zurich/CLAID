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
            static bool fileExists(std::string path);

            static bool createDirectoriesRecursively(std::string path);
            static bool createDirectory(std::string path);
            static bool removeDirectoryRecursively(const std::string& path);

            static bool getAllDirectoriesInDirectory(std::string path, std::vector<std::string>& output);

            static bool readFileToString(std::string path, std::string& content);

            static bool removeFile(const std::string& path);
            static bool removeFileIfExists(const std::string& path);

    };
}
