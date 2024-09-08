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

#include "dispatch/core/shared_queue.hh"
#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/RemoteDispatching/ClientTable.hh"
#include "dispatch/core/Utilities/Time.hh"
#include "dispatch/core/RemoteDispatching/TLSClientKeyStore.hh"

#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>

#include <thread>

using claidservice::DataPackage;
using claidservice::ControlPackage;
using claidservice::CtrlType;
using claidservice::RemoteClientInfo;

namespace claid
{
    class RemoteDispatcherClient
    {
      
        public: 
            RemoteDispatcherClient(const std::string& addressToConnectTo,
                    const std::string& host,
                    const std::string& userToken,
                    const std::string& deviceID,
                    ClientTable& clientTable);
            
            void shutdown();
            virtual ~RemoteDispatcherClient();

            absl::Status start();
            absl::Status start(const TLSClientKeyStore& clientKeyStore);

            bool isConnected() const;
            absl::Status getLastStatus() const;

            bool isRunning() const;

        private:
            void processReading();
            void processWriting();
            void processPacket(DataPackage& pkt);
            
            void connectAndMonitorConnection();

            void onConnectedToServer();
            void onDisconnectedFromServer();


        private:

            // Current host (i.e., the identifier of the current instance/configuration of CLAID).
            const std::string host;
            const std::string userToken;
            const std::string deviceID;

            bool connected = false;
            bool connectionMonitorRunning = false;
            absl::Status lastStatus;

            Time lastTimePackageWasSent;

            static Duration MAX_TIME_WITHOUT_PACKAGE_BEFORE_TESTING_TIMEOUT;

            ClientTable& clientTable;

           
            std::shared_ptr<grpc::Channel> grpcChannel;
            std::unique_ptr< claidservice::ClaidRemoteService::Stub> stub;
            
            std::shared_ptr<grpc::ClientContext> streamContext;
            std::shared_ptr<grpc::ClientReaderWriter<claidservice::DataPackage, claidservice::DataPackage>> stream;

            // Thread that establishes and monitors the connection to the remote server.
            // If connected, this thread reads packages.
            // Upon disconnet, the reading stops and the thread tries to reconnect.
            std::unique_ptr<std::thread> watcherAndReaderThreader;
            std::unique_ptr<std::thread> writeThread;

            bool useTLS = false;
            TLSClientKeyStore clientKeyStore;

    };
}