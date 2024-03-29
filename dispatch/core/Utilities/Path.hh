
#pragma once

#include <string>
#include "dispatch/core/Utilities/Time.hh"
#include "dispatch/core/Utilities/StringUtils.hh"

namespace claid
{
    class Path
    {
        private:
            std::string path;

            size_t getIndexOfRightMostPathSeparator() const
            {
                if(path.size() == 0)
                {
                    return 0;
                }

                size_t index = path.size() - 1;

                while(index > 0)
                {
                    if(this->path[index] == '/' || this->path[index] == '\\')
                    {
                        return index;
                    }

                    index--;
                }

                return 0;
            }

        public:

            Path()
            {

            }

            Path(std::string path) : path(path)
            {

            }


            static Path join(const std::string& path1, const std::string& path2)
            {
                return Path(path1 + "/" + path2);
            }

            static Path join(const std::string& path1, const std::string& path2, const std::string& path3)
            {
                return Path(path1 + "/" + path2 + "/" + path3);
            }

            std::string getFolderPath() const
            {
                size_t index = this->getIndexOfRightMostPathSeparator();
                if(index == 0)
                {
                    return "";
                }

                return this->path.substr(0, index);
            }

            std::string getFilePath() const
            {
                size_t index = this->getIndexOfRightMostPathSeparator();
                if(index == 0)
                {
                    // No path separator found, assume the path points to a file.
                    return this->path;
                }
                return this->path.substr(index + 1, this->path.size());
            }

            std::string getFileSuffix() const
            {
                if(path.size() == 0)
                {
                    return "";
                }

                size_t index = path.size() - 1;
            
                while(index > 0)
                {
                    if(this->path[index] == '.')
                    {
                        return this->path.substr(index + 1, this->path.size());
                    }

                    index--;
                }

                return "";
            }

            std::string getPathRelativeTo(const std::string& path) const
            {
                if(StringUtils::startsWith(this->path, path))
                {
                    return this->path.substr(path.size() + 1, this->path.size());
                }
                return path;
            }

            std::string toString() const
            {
                return this->path;
            }

            operator std::string() const
            {
                return this->path;
            }

            bool operator==(const Path& rhs) const
            {
                return this->toString() == rhs.toString();
            }

            bool operator!=(const Path& rhs) const
            {
                return this->toString() != rhs.toString();
            }

            Path& operator=(const std::string& path)
            {
                this->path = path;
                return *this;
            }

            // Splits a/b/c/d.txt into a/b/c and d.txt
            static void splitPathIntoFolderAndFileName(const std::string& path, std::string& folderPath, std::string& fileName)
            {
                if(path.find("/") == std::string::npos)
                {
                    // No folder, assume path is only a fileName.
                    folderPath = "";
                    fileName = path;
                    return;
                }
                int index = path.size() - 1;
                while(index > 0)
                {
                    if(path[index] == '/')
                    {
                        folderPath = path.substr(0, index);
                        fileName = path.substr(index + 1, path.size());
                        return;
                    }
                    index--;
                }
            }

    };
}