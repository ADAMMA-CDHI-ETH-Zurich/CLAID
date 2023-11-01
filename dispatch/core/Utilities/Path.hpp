/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions
* 
* Authors: Patrick Langer
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/
#pragma once

#include <string>
#include "Utilities/Time.hpp"
#include "Utilities/StringUtils.hpp"

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


    };
}