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

#ifndef SYNC_UTIL_HH_
#define SYNC_UTIL_HH_

#include <list>
#include <memory>
#include <mutex>

namespace claid {

template <class T>
class SharedQueue {
  public:
    void push_front(std::shared_ptr<T> val);  // adds in the front 
    void push_back(std::shared_ptr<T> val); // adds at the end 
    std::shared_ptr<T> pop_front();  // returns null_ptr if empty 

  private:
    std::mutex cMutex; 
    std::list<std::shared_ptr<T>> container;  // stores the elements 
};

template <class T>
void SharedQueue<T>::push_front(std::shared_ptr<T> val) {
    std::lock_guard<std::mutex> lock(cMutex);
    container.push_front(val);
}

template <class T>
void SharedQueue<T>::push_back(std::shared_ptr<T> val) {
   std::lock_guard<std::mutex> lock(cMutex);
   container.push_back(val);
}

template <class T>
std::shared_ptr<T> SharedQueue<T>::pop_front() {
    std::lock_guard<std::mutex> lock(cMutex);
    if (!container.empty()) {
        std::shared_ptr<T> ret = container.front();
        container.pop_front();
        return ret; 
    }
    return std::shared_ptr<T>(nullptr); 
}

}  // namespace claid  

#endif  // SYNC_UTIL_HH
