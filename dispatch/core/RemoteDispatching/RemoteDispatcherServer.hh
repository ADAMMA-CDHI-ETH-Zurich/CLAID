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

#pragma once

#include <string>

#include <grpc/grpc.h>
#include <grpcpp/server.h>

#include "dispatch/core/RemoteDispatching/RemoteService.hh"

#include "absl/strings/str_split.h"
#include "absl/status/status.h"

#include "dispatch/core/RemoteDispatching/HostUserTable.hh"

namespace claid
{
  class RemoteDispatcherServer
  {
    public:
      RemoteDispatcherServer(const std::string& addr, HostUserTable& hostUserTable);
      
      virtual ~RemoteDispatcherServer();

      absl::Status start();
      void shutdown();

      bool isRunning() const;

    private:
      void buildAndStartServer();

    private:
      const std::string addr;

      bool running = false;

      RemoteServiceImpl remoteServiceImpl;
      std::unique_ptr<grpc::Server> server;
  };

}
