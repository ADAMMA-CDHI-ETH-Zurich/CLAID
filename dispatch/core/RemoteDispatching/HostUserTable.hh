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
    HostUserTable();
    virtual ~HostUserTable() {}

    inline std::shared_ptr<SharedQueue<claidservice::DataPackage>> inputQueue() { return fromClientsQueue; }

    // Looks up the output queue for the host of a specific user (i.e., address (host, user))
    // Other hosts connected to the current host can either be servers or clients.
    // If it are clients, that means multiple instances of the same host connect, but with a different user id (multiple user ids, same host).
    // This function looks up the queue for one specific user connected to the current host.
    absl::Status lookupOutputQueueForHostUser(const std::string& host, const std::string& userToken, std::shared_ptr<SharedQueue<claidservice::DataPackage>>& queue);

    // Looks up output queue for all users running the same host (i.e., address host:*)
    absl::Status lookupOutputQueuesForHost(const std::string& host, std::vector<std::shared_ptr<SharedQueue<claidservice::DataPackage>>>& queues);

    // Typically called when a user connects to the current host (i.e., the current instance of CLAID).
    // Registers the user with the current server.
    absl::Status addRemoteClient(const std::string& host, const std::string& userToken, const std::string& deviceID);
    absl::Status removeRemoteClient(const std::string& host, const std::string& userToken, const std::string& deviceID);

  private:
    
    // Queue where the clients output their received packages to.
    std::shared_ptr<SharedQueue<claidservice::DataPackage>> fromClientsQueue;

    // Maps <host, user> to the corresponding queue.
    // Each queue will be used by a corresponding RemoteClientHandler.
    HostUserQueueMap hostUserQueueMap;

    // Multiple users can run the same host.
    // This map stores connected users for each host.
    // In other words, you can use this map to find out 
    // which *connected* users run a certain host.
    std::map<std::string, std::vector<std::string>> hostToUserMap;
    
    // Recursive_mutex, because a calling thread might 
    // make sub calls to lookupOutputQueueForHostUser e.g. from the lookupOutputQueuesForHost function.
    std::recursive_mutex hostUserTableMutex;



    
};

}