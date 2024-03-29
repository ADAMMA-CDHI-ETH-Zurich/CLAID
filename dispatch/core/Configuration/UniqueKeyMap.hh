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

#include <map>
#include "absl/strings/str_split.h"
#include "absl/status/status.h"

namespace claid
{
    // Wrapper around std::map that ensures enforces that each key is only added once when using insert.
    template<typename T>
    struct UniqueKeyMap
    {
        private:
            std::map<std::string, T> map;

        public:
            absl::Status insert(const std::pair<std::string, T> entry)
            {
                auto result = map.insert(entry);
                bool existedBefore = !result.second;
                if(existedBefore)
                {
                    return absl::AlreadyExistsError(
                            absl::StrCat("Cannot insert key into UnqiueKeyMap: Key \"", entry.first, "\" already exists."));
                }
                return absl::OkStatus();
            }

            bool exists(const std::string& key) const
            {
                return map.find(key) != map.end();
            }

            bool empty() const
            {
                return map.empty();
            }

            T& operator[](const std::string& key)
            {
                return map[key];
            }

            const T& operator[](const std::string& key) const
            {
                return map[key];
            }

            // Iterator for non-const map
            typename std::map<std::string, T>::iterator begin() {
                return map.begin();
            }

            typename std::map<std::string, T>::iterator end() {
                return map.end();
            }

            // Iterator for const map
            typename std::map<std::string, T>::const_iterator begin() const {
                return map.begin();
            }

            typename std::map<std::string, T>::const_iterator end() const {
                return map.end();
            }

             // Find a key in the map
            typename std::map<std::string, T>::iterator find(const std::string& key) {
                return map.find(key);
            }

            typename std::map<std::string, T>::const_iterator find(const std::string& key) const {
                return map.find(key);
            }

            // Clear all elements in the map
            void clear() {
                map.clear();
            }

            size_t size() const
            {
                return map.size();
            }
    };
}