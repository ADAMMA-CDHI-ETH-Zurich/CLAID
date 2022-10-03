#pragma once

#include <string>
#include <vector>

namespace portaible
{
    class FileUtils
    {
        public:
            static bool dirExists(std::string path);
            static bool fileExists(std::string path);

            static bool createDirectoriesRecursively(std::string path);
            static bool createDirectory(std::string path);

            static bool getAllDirectoriesInDirectory(std::string path, std::vector<std::string>& output);

            static bool readFileToString(std::string path, std::string& content);

    };
}
