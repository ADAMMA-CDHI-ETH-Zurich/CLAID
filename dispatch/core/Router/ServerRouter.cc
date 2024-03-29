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

#include "dispatch/core/Router/ServerRouter.hh"
#include "dispatch/core/Logger/Logger.hh"
#include <sstream>

namespace claid
{
    ServerRouter::ServerRouter(const std::string& currentHost, 
        const RoutingTree& routingTree, 
        HostUserTable& hostUserTable) : currentHost(currentHost), routingTree(routingTree), hostUserTable(hostUserTable)
    {

    }

    absl::Status ServerRouter::routePackage(std::shared_ptr<DataPackage> dataPackage) 
    {

        const std::string& sourceHost = dataPackage->source_host();
        const std::string& sourceModule = dataPackage->source_module();

        const std::string& targetHost = dataPackage->target_host();
        const std::string& targetModule = dataPackage->target_module();
        Logger::logInfo("Test");
        Logger::logInfo("ServerRouter routing package from host \"%s\" (Module \"%s\"), "
                        "destined for host \"%s\" (Module \"%s\").", sourceHost.c_str(), sourceModule.c_str(), targetHost.c_str(), targetModule.c_str());


        if(!canReachHost(targetHost))
        {
            return absl::InvalidArgumentError(absl::StrCat(
                "Error in ServerRouter: Cannot route package from source host \"", sourceHost, "\" ",
                "to target host \"", targetHost, "\". The target host is not connected as client to the source host."
            ));
        }

        // The function canReachHost will automatically cache the route to the targetHost in our routingTable.
        // Hence, it is assured that the routingTable will have an entry for targetHost.
        const std::vector<std::string>& route = this->routingTable[targetHost];

        if(route.size() == 0)
        {
            return absl::NotFoundError(absl::StrCat(
                "ServerRouter on host \"", currentHost, "\" failed to route received package from host \"", sourceHost, "\" to host \"", targetHost, "\".\n",
                "No route found."
            ));
        }

        absl::Status status;
        const std::string& nextHost = route[0];

        // This means that the targetHost is a direct client of us.
        if(route.size() == 1)
        {
            // If the target user_token is set to *, we route to all users.
            // If not, only to a specific user.
            bool routeToAllUsers = dataPackage->target_user_token() == "";

            if(routeToAllUsers)
            {
                Logger::logInfo("ServerRouter routing to all users");
                std::vector<std::shared_ptr<SharedQueue<DataPackage>>> queues;

                status = this->hostUserTable.lookupOutputQueuesForHost(nextHost, queues);

                if(!status.ok() || queues.size() == 0)
                {
                    std::stringstream ss;
                    ss << status;
                    Logger::logWarning("ServerRouter failed to route package from host \"%s\" to host \"%s\", "
                    "Error was: ", sourceHost.c_str(), targetHost.c_str(), ss.str().c_str());

                    return absl::OkStatus();
                }

                for(std::shared_ptr<SharedQueue<DataPackage>> queue : queues)
                {
                    Logger::logInfo("ServerRouter routing to all users, inserting to queue");
                    queue->push_back(dataPackage);
                }

            }
            else
            {

                const std::string& targetUserToken = dataPackage->target_user_token();
                std::shared_ptr<SharedQueue<DataPackage>> queue;
                status = this->hostUserTable.lookupOutputQueueForHostUser(nextHost, targetUserToken, queue);

                if(!status.ok())
                {
                    std::stringstream ss;
                    ss << status;
                    Logger::logWarning("ServerRouter failed to route package from host \"%s\" to user \"%s\" running host \"%s\", "
                    "Error was: %s", sourceHost.c_str(), targetUserToken.c_str(), targetHost.c_str(), ss.str().c_str());

                    return absl::OkStatus();
                }

                Logger::logInfo("ServerRouter routing to one user, inserting to queue");
                queue->push_back(dataPackage);
            }

        }   
        // There are still multiple hosts to go between us and the target host. 
        // In that case, the next host in the list HAS to be a server, and there can only be ONE instance of that host.
        else
        {
            std::vector<std::shared_ptr<SharedQueue<DataPackage>>> queues;
            status = this->hostUserTable.lookupOutputQueuesForHost(nextHost, queues);

            if(!status.ok())
            {
                Logger::logWarning("ServerRouter failed to route package from host \"%s\" to host \"%s\", "
                "via intermediate host \"%s\". No instance of the intermediate host \"%s\" is currently connected or was connected before.\n"
                "Package will be discarded.", sourceHost.c_str(), targetHost.c_str(), nextHost.c_str(), nextHost.c_str());

                return absl::OkStatus();
            }

            if(queues.size() > 1)
            {
                return absl::InvalidArgumentError(absl::StrCat(
                    "Amiguity detected in ServerRouter: Trying to route package from host \"", sourceHost, "\" ",
                    "to target host \"", targetHost, "\" via Server host \"", nextHost, "\". However, multiple instances\n",
                    " of host \"", nextHost, "\" are connected. The host \"", nextHost, "\" is an intermediate node ",
                    " and therefore only one instance of that host should exist."
                ));
            }
            Logger::logInfo("ServerRouter routing to intermediate server, inserting to queue");

            queues[0]->push_back(dataPackage);

        }
        Logger::logInfo("ServerRouter routed package successfully");
        return absl::OkStatus();
    }

    bool ServerRouter::canReachHost(const std::string& hostname)
    {
        // Check if we have seen this host already before. 
        // If yes, and we can reach it, we have cached the routing path to that host in our routing table.
        auto it = this->routingTable.find(hostname);
        if(it != this->routingTable.end())
        {
            return true;
        }

        std::vector<std::string> route;
        RoutingDirection direction;

        // Returns false if no route was found.
        if(!routingTree.getRouteFromHostToHost(this->currentHost, hostname, route, direction))
        {
            return false;
        }

        // We are a ServerRouter and route packages to clients that are connected to us.
        // Hence, we can only route downwards in the routing tree.
        if(direction != ROUTE_DOWN)
        {
            return false;
        }

        // Cache result for later routing.
        this->routingTable[hostname] = route;
        return true;
    }

}