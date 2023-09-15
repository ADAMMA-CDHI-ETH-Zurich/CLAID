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
    };
}