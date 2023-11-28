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

            absl::Status start();

            bool isConnected() const;
            absl::Status getLastStatus() const;

        private:
            void processReading();
            void processWriting();
            
            void connectAndMonitorConnection();
            


        private:

            // Current host (i.e., the identifier of the current instance/configuration of CLAID).
            const std::string host;
            const std::string userToken;
            const std::string deviceID;

            bool connected = false;
            bool connectionMonitorRunning = false;
            absl::Status lastStatus;


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

    };
}