#pragma once

#include <string>
#include "Utilities/Time.hpp"


namespace claid
{
    class Path
    {
        private:
            std::string path;

            size_t getIndexOfRightMostPathSeparator()
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

            std::string getFolderPath()
            {
                size_t index = this->getIndexOfRightMostPathSeparator();
                if(index == 0)
                {
                    return "";
                }

                return this->path.substr(0, index);
            }

            std::string getFilePath()
            {
                size_t index = this->getIndexOfRightMostPathSeparator();
                if(index == 0)
                {
                    // No path separator found, assume the path points to a file.
                    return this->path;
                }
                return this->path.substr(index + 1, this->path.size());
            }

            std::string getFileSuffix()
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


    };
}