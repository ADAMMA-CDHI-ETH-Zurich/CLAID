/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
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