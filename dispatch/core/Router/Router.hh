#pragma once

#include "dispatch/core/CLAIDConfig/CLAIDConfig.hh"
#include "dispatch/proto/claidservice.pb.h"
#include "dispatch/core/Router/DispatcherStub.hh"
#include "dispatch/core/Router/RoutingNode.hh"
#include "dispatch/core/Router/RoutingTree.hh"
#include "dispatch/core/shared_queue.hh"

#include "absl/strings/str_split.h"

#include <map>
#include <thread>

using claidservice::CLAIDConfig;
using claidservice::HostConfig;
using claidservice::DataPackage;



namespace claid
{
    // A Router has one input queue and N output queues.
    // The input queue is implemented by this Router base class, the output queues are realized separately by the LocalRouter, ClienRouter and ServerRouter classes.
    // A Router runs in a separate thread and waits for data packages to arrive on the input queue. It then forwards the package to the output queue accordingly.
    class Router
    {
        protected:
            bool running = false;
            SharedQueue<claidservice::DataPackage>& incomingQueue;
            std::unique_ptr<std::thread> routingThread;

        protected:

            virtual absl::Status initialize();
            virtual void routePackage(std::shared_ptr<DataPackage> package);

            void runRouting();


        public:
            Router(SharedQueue<claidservice::DataPackage>& incomingQueue) : incomingQueue(incomingQueue)
            {
            }

            virtual ~Router() {}

            absl::Status start() ;
            void enqueuePackageForRouting(std::shared_ptr<DataPackage> dataPackage);

            
            
    };

    inline absl::Status getTargetHostAndModule(const DataPackage& package, std::string& host, std::string& module)
    {
        std::vector<std::string> tokens = absl::StrSplit(package.target_host_module(), ":");
        if(tokens.size() != 2)
        {
            return absl::InvalidArgumentError(absl::StrCat("Unable to parse \"", package.target_host_module(), "\" into host and module.",
                    "Please make sure the string follows the format host:module (exactly one colon is expected)."));
        }

        host = tokens[0];
        module = tokens[1];
        return absl::OkStatus();
    }

    inline void Router::getTargetUser(const DataPackage& package, std::string& user)
    {
        user = package.target_user_token();    
    }
}