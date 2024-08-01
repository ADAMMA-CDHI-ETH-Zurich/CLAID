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

            static bool getFileSize(const std::string& path, uint64_t& size);
            

    };
}
