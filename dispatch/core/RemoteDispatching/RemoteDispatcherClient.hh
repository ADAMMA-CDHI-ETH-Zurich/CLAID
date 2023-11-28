#pragma once

#include "dispatch/core/shared_queue.hh"
#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/RemoteDispatching/ClientTable.hh"

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

            absl::Status registerAtServerAndStartStreaming();

        private:
            void processReading();
            void processWriting();
            
        private:

            // Current host (i.e., the identifier of the current instance/configuration of CLAID).
            const std::string host;
            const std::string userToken;
            const std::string deviceID;

            bool running;

            ClientTable& clientTable;

           
            std::shared_ptr<grpc::Channel> grpcChannel;
            std::unique_ptr< claidservice::ClaidRemoteService::Stub> stub;
            
            std::shared_ptr<grpc::ClientContext> streamContext;
            std::shared_ptr<grpc::ClientReaderWriter<claidservice::DataPackage, claidservice::DataPackage>> stream;
            std::unique_ptr<std::thread> readThread;
            std::unique_ptr<std::thread> writeThread;

    };
}