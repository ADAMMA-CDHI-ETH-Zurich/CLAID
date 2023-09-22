#pragma once

#include "dispatch/core/shared_queue.hh"


#include <memory>
#include <set>
#include <shared_mutex>
#include <string>

#include <grpc/grpc.h>

#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/shared_queue.hh"
#include "dispatch/core/RemoteDispatching/RemoteClientKey.hh"


namespace claid
{

typedef std::map<RemoteClientKey, std::shared_ptr<SharedQueue<claidservice::DataPackage>>> HostUserQueueMap;

// Just as the ModuleTable manages the queues for all local Modules across the available runtimes,
// The HostUserTable manages the queues for all connected Hosts, possibly representing different users.
// In other words, the HostUserTable has a queue for each connected user. 
// Multiple users, however, can run the same host (e.g., multiple participants of a study running the "android_app_host").
class HostUserTable 
{
  public:
    virtual ~HostUserTable() {}

    inline SharedQueue<claidservice::DataPackage>& inputQueue() { return fromClientsQueue; }

    // Looks up the output queue for the host of specific user (i.e., address host:user)
    absl::Status lookupOutputQueueForHostUser(const std::string& host, const std::string& userToken, std::shared_ptr<SharedQueue<claidservice::DataPackage>>& queue);

    // Looks up output queue for all users running the same host (i.e., address host:*)
    absl::Status lookupOutputQueuesForHost(const std::string& host, std::vector<std::shared_ptr<SharedQueue<claidservice::DataPackage>>>& queues);

    absl::Status addRemoteClient(const std::string& host, const std::string& userToken, const std::string& deviceID);
    absl::Status removeRemoteClient(const std::string& host, const std::string& userToken, const std::string& deviceID);

  private:
    
    // Queue where the clients output their received packages to.
    SharedQueue<claidservice::DataPackage> fromClientsQueue;

    // Maps <host, user> to the corresponding queue.
    // Each queue will be used by a corresponding RemoteClientHandler.
    HostUserQueueMap hostUserQueueMap;

    // Multiple users can run the same host.
    // This map stores connected users for each host.
    // In other words, you can use this map to find out 
    // which connected users run a certain host.
    std::map<std::string, std::vector<std::string>> hostToUserMap;
    
    std::mutex hostUserTableMutex;



    
};

}