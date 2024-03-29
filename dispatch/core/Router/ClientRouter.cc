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

#include "dispatch/core/Router/ClientRouter.hh"

#include "absl/status/status.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_cat.h"
#include "dispatch/core/proto_util.hh"
namespace claid {
    
    ClientRouter::ClientRouter(const std::string& currentHost,
                const RoutingTree& routingTree, ClientTable& clientTable) : currentHost(currentHost), routingTree(routingTree), clientTable(clientTable)
    {
        
    }

    absl::Status ClientRouter::routePackage(std::shared_ptr<DataPackage> dataPackage)
    {
        const std::string& sourceHost = dataPackage->source_host();
        const std::string& sourceModule = dataPackage->source_module();

        const std::string& targetHost = dataPackage->target_host();
        const std::string& targetModule = dataPackage->target_module();

        Logger::logInfo("ClientRouter routing package from host \"%s\" (Module \"%s\"), "
                        "destined for host \"%s\" (Module \"%s\"):\n%s.", sourceHost.c_str(), sourceModule.c_str(), targetHost.c_str(), targetModule.c_str(),
                        messageToString(*dataPackage).c_str());

        if(!canReachHost(targetHost))
        {
            return absl::InvalidArgumentError(absl::StrCat(
                "ClientRouter on host \"", currentHost, "\" failed to route package from host \"", sourceHost, "\" ",
                "to target host \"", targetHost, "\". There is no route from host \"", currentHost, "\" to host \"", targetHost, "\"."
            ));
        }

        this->clientTable.getToRemoteClientQueue().push_back(dataPackage);

        return absl::OkStatus();
    }

    bool ClientRouter::canReachHost(const std::string& hostname)
    {
        std::vector<std::string> route;
        RoutingDirection direction;

        // Returns false if no route was found.
        if(!routingTree.getRouteFromHostToHost(this->currentHost, hostname, route, direction))
        {
            return false;
        }


        // We are a ClientRouter and route packages to a server we are connected to.
        // Hence, we can only route upwards in the routing tree.
        return direction == ROUTE_UP;
    }

}