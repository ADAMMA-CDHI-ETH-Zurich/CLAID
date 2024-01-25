#pragma once

#include "dispatch/proto/claidservice.pb.h"
#include "dispatch/core/Router/RoutingNode.hh"
#include "dispatch/core/Router/RoutingTree.hh"
#include "dispatch/core/shared_queue.hh"
#include "dispatch/core/Logger/Logger.hh"

#include "absl/strings/str_split.h"
#include "absl/status/status.h"

#include <map>
#include <thread>

using claidservice::HostConfig;
using claidservice::DataPackage;



namespace claid
{
    // A Router takes in data packages and routes it to N output queues.
    // The incoming package might be routed to only one, multiple or all output queues.
    class Router
    {
        public:
            Router() {}
            virtual ~Router() {}

            virtual absl::Status start();
            virtual absl::Status routePackage(std::shared_ptr<DataPackage> package) = 0;
            virtual bool canReachHost(const std::string& host) = 0;
    };

    inline absl::Status getTargetHostAndModule(const DataPackage& package, std::string& host, std::string& module)
    {
        std::vector<std::string> tokens = absl::StrSplit(package.target_module(), ":");
        if(tokens.size() != 2)
        {
            return absl::InvalidArgumentError(absl::StrCat("Unable to parse \"", package.target_module(), "\" into host and module.",
                    "Please make sure the string follows the format host:module (exactly one colon is expected)."));
        }

        host = tokens[0];
        module = tokens[1];
        return absl::OkStatus();
    }

    inline void getTargetUser(const DataPackage& package, std::string& user)
    {
        user = package.target_user_token();    
    }
}