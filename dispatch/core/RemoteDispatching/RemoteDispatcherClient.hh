#pragma once

#include "dispatch/core/shared_queue.hh"
#include "dispatch/proto/claidservice.grpc.pb.h"

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
                    SharedQueue<DataPackage>& incomingQueue, 
                    SharedQueue<DataPackage>& outgoingQueue);
            
            void shutdown();
            virtual ~RemoteDispatcherClient() { shutdown(); };

            absl::Status registerAtServerAndStartStreaming();

        private:
            void processReading();
            void processWriting();
            
        private:

            // Current host (i.e., the identifier of the current instance/configuration of CLAID).
            const std::string host;
            const std::string userToken;
            const std::string deviceID;

            // Incoming from server -> router (packages we receive from external connection).
            SharedQueue<claidservice::DataPackage>& incomingQueue;

            // Incoming from router -> server (packages we send to external connection).
            SharedQueue<claidservice::DataPackage>& outgoingQueue;
            std::shared_ptr<grpc::Channel> grpcChannel;
            std::unique_ptr< claidservice::ClaidRemoteService::Stub> stub;
            
            std::shared_ptr<grpc::ClientContext> streamContext;
            std::shared_ptr<grpc::ClientReaderWriter<claidservice::DataPackage, claidservice::DataPackage>> stream;
            std::unique_ptr<std::thread> readThread;
            std::unique_ptr<std::thread> writeThread;

    };
}