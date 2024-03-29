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
#include <grpc/grpc.h>
#include <thread>


#include "dispatch/core/shared_queue.hh"
#include "dispatch/proto/claidservice.pb.h"
#include "dispatch/proto/claidservice.grpc.pb.h"


#include "absl/strings/str_split.h"
#include "absl/status/status.h"

using claidservice::DataPackage;



namespace claid
{
    // Just as the RuntimeDispatcher manages local DispatcherClients,
    // the RemoteClientHandler manages connected RemoteDispatcherClients.
    // It reads and writes messages from/to the associated RemoteDispatcherClient.
    class RemoteClientHandler
    {
        explicit RemoteClientHandler(SharedQueue<claidservice::DataPackage>& inQueue,
                                    SharedQueue<claidservice::DataPackage>& outQueue,
                                    const std::string& userToken, const std::string& deviceID);

        void shutdownWriterThread();
        bool alreadyRunning();
        grpc::Status startWriterThread(grpc::ServerReaderWriter<claidservice::DataPackage, claidservice::DataPackage>* stream);
        grpc::Status processReading(grpc::ServerReaderWriter<claidservice::DataPackage, claidservice::DataPackage>* stream);

        void shutdown();

        bool sendPingToClient();

    private:
        void processWriting(grpc::ServerReaderWriter<claidservice::DataPackage, claidservice::DataPackage>* stream);
        void processPacket(claidservice::DataPackage& pkt, grpc::Status& status);
    
    private:

        // Incoming from client -> router (packages we receive from external connection, i.e., connected client).
        SharedQueue<claidservice::DataPackage>& incomingQueue;

        // Outgoing from router -> client (packages we send to external connection, i.e., connected client).
        SharedQueue<claidservice::DataPackage>& outgoingQueue;

        grpc::ServerReaderWriter<claidservice::DataPackage, claidservice::DataPackage>* writeStream = nullptr;

        std::mutex writeThreadMutex; // protects the write thread
        std::mutex pingMutex;
        std::unique_ptr<std::thread> writeThread;

        const std::string userToken;
        const std::string deviceID;

        bool active = false;

        friend class RemoteServiceImpl;
    };

}