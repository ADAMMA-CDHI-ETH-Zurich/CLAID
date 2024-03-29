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

#include "dispatch/core/RemoteDispatching/HostUserTable.hh"
#include "dispatch/core/Logger/Logger.hh"

namespace claid
{

HostUserTable::HostUserTable() : fromClientsQueue(std::make_shared<SharedQueue<DataPackage>>())
{

}

// Looks up the output queue for the host of specific user (i.e., address host:user)
absl::Status HostUserTable::lookupOutputQueueForHostUser(const std::string& host, 
    const std::string& userToken, std::shared_ptr<SharedQueue<DataPackage>>& queue)
{
    std::lock_guard<std::recursive_mutex> lock(this->hostUserTableMutex);
    RemoteClientKey key = makeRemoteClientKey(host, userToken);

    auto it = this->hostUserQueueMap.find(key);
    if(it == this->hostUserQueueMap.end())
    {
        return absl::NotFoundError(absl::StrCat(
            "HostUserTable unable to find queue for user \"", userToken, "\" on host \"", host, "\".\n",
            "The client was not found in the host user map."
        ));
    }

    queue = it->second;
    return absl::OkStatus();
}

// Looks up output queue for all users running the same host (i.e., address host:*)
absl::Status HostUserTable::lookupOutputQueuesForHost(const std::string& host, 
    std::vector<std::shared_ptr<SharedQueue<DataPackage>>>& queues)
{
    queues.clear();
        Logger::logInfo("T14");

    std::lock_guard<std::recursive_mutex> lock(this->hostUserTableMutex);

    auto it = this->hostToUserMap.find(host);
    if(it == this->hostToUserMap.end())
    {
        return absl::NotFoundError(absl::StrCat(
            "HostUserTable failed to lookup output queues for all users with host \"", host, "\".\n",
            "Host was not found and possibly has not been registered yet."
        ));
    }
        Logger::logInfo("T15");

    const std::vector<std::string>& listOfUsers = it->second;

    for(const std::string& user : listOfUsers)
    {
                Logger::logInfo("T16");

        std::shared_ptr<SharedQueue<DataPackage>> queue;
                        Logger::logInfo("T17");

        absl::Status status = this->lookupOutputQueueForHostUser(host, user, queue);
                        Logger::logInfo("T18");

        if(!status.ok())
        {
            return status;
        }

        queues.push_back(queue);
    }

    return absl::OkStatus();
}

absl::Status HostUserTable::addRemoteClient(const std::string& host, const std::string& userToken, const std::string& deviceID)
{
    std::lock_guard<std::recursive_mutex> lock(this->hostUserTableMutex);
        Logger::logInfo("hostusrtable 1");

    RemoteClientKey key = makeRemoteClientKey(host, userToken);
            Logger::logInfo("hostusrtable 2");

    auto it = this->hostUserQueueMap.find(key);

    if(it != this->hostUserQueueMap.end())
    {
        return absl::AlreadyExistsError(absl::StrCat(
            "HostUserTable cannot add remote client \"", host, ":", userToken, "\".\n",
            "A user with these identifiers was already registered before."
        ));
    }

    std::shared_ptr<SharedQueue<DataPackage>> queue = std::make_shared<SharedQueue<DataPackage>>();
    this->hostUserQueueMap.insert(std::make_pair(key, queue));
    std::vector<std::string>& listOfUsers = this->hostToUserMap[host];
        Logger::logInfo("hostusrtable 4");

    auto it2 = std::find(listOfUsers.begin(), listOfUsers.end(), userToken);
    if(it2 != listOfUsers.end())
    {
        return absl::AlreadyExistsError(absl::StrCat(
            "HostUserTable failed to insert user \"", userToken, "\" for to list of users for host \"", host, "\".\n",
            "This user was already registered before."
        ));
    }
    listOfUsers.push_back(userToken);
        Logger::logInfo("hostusrtable 5");

    return absl::OkStatus();
}

absl::Status HostUserTable::removeRemoteClient(const std::string& host, const std::string& userToken, const std::string& deviceID)
{
    std::lock_guard<std::recursive_mutex> lock(this->hostUserTableMutex);

    RemoteClientKey key = makeRemoteClientKey(host, userToken);

    {
        auto it = this->hostUserQueueMap.find(key);

        if(it == this->hostUserQueueMap.end())
        {
            return absl::NotFoundError(absl::StrCat(
                "HostUserTable cannot remove remote client \"", host, ":", userToken, "\".\n",
                "A user with these identifiers was not registered."
            ));
        }
        this->hostUserQueueMap.erase(it);
    }


    auto it = this->hostToUserMap.find(host);

    if(it == this->hostToUserMap.end())
    {
        return absl::NotFoundError(absl::StrCat(
            "HostUserTable failed to remove client.\n",
            "The host \"", host, "\" of user \"", userToken, "\" was not registered, but should have been."
        ));
    }
    
    std::vector<std::string>& listOfUsers = it->second;
    // Remove user from the list of users for that host. 

    auto it2 = std::find(listOfUsers.begin(), listOfUsers.end(), userToken);
    if(it2 == listOfUsers.end())
    {
        return absl::NotFoundError(absl::StrCat(
            "HostUserTable failed to remove user \"", userToken, "\" from list of users of host \"", host, "\".\n",
            "The user was not found or not registered before."
        ));
    }

    listOfUsers.erase(it2);
    return absl::OkStatus();
}

}
