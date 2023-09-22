#include "dispatch/core/RemoteDispatching/HostUserTable.hh"


namespace claid
{
// Looks up the output queue for the host of specific user (i.e., address host:user)
absl::Status HostUserTable::lookupOutputQueueForHostUser(const std::string& host, 
    const std::string& userToken, std::shared_ptr<SharedQueue<DataPackage>>& queue)
{
    std::lock_guard<std::mutex> lock(this->hostUserTableMutex);
    RemoteClientKey key = makeRemoteClientKey(host, userToken);

    auto it = this->hostUserQueueMap.find(key);
    if(it == this->hostUserQueueMap.end())
    {
        return absl::NotFoundError(absl::StrCat(
            "HostUserTable unable to find queue for client \"", host, ":", userToken, "\".\n",
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

    std::lock_guard<std::mutex> lock(this->hostUserTableMutex);

    auto it = this->hostToUserMap.find(host);
    if(it == this->hostToUserMap.end())
    {
        return absl::NotFoundError(absl::StrCat(
            "HostUserTable failed to lookup output queues for all users with host \"", host, "\".\n",
            "Host was not found and possibly has not been registered yet."
        ));
    }

    const std::vector<std::string>& listOfUsers = it->second;

    for(const std::string& user : listOfUsers)
    {
        std::shared_ptr<SharedQueue<DataPackage>> queue;
        absl::Status status = this->lookupOutputQueueForHostUser(host, user, queue);
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
    std::lock_guard<std::mutex> lock(this->hostUserTableMutex);

    RemoteClientKey key = makeRemoteClientKey(host, userToken);
    
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

    auto it2 = std::find(listOfUsers.begin(), listOfUsers.end(), userToken);
    if(it2 != listOfUsers.end())
    {
        return absl::AlreadyExistsError(absl::StrCat(
            "HostUserTable failed to insert user \"", userToken, "\" for to list of users for host \"", host, "\".\n",
            "This user was already registered before."
        ));
    }
    listOfUsers.push_back(userToken);

    return absl::OkStatus();
}

absl::Status HostUserTable::removeRemoteClient(const std::string& host, const std::string& userToken, const std::string& deviceID)
{
    std::lock_guard<std::mutex> lock(this->hostUserTableMutex);

    RemoteClientKey key = makeRemoteClientKey(host, userToken);

    {
        auto it = this->hostUserQueueMap.find(key);

        if(it == this->hostUserQueueMap.end())
        {
            return absl::AlreadyExistsError(absl::StrCat(
                "HostUserTable cannot remove remote client \"", host, ":", userToken, "\".\n",
                "A user with these identifiers was nnot registered."
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
