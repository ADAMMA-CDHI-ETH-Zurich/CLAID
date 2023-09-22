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
    // It reads and messages from/to the associated RemoteDispatcherClient.
    class RemoteClientHandler
    {
        explicit RemoteClientHandler(SharedQueue<claidservice::DataPackage>& inQueue,
                                    SharedQueue<claidservice::DataPackage>& outQueue,
                                    const std::string& userToken, const std::string& deviceID);

        void shutdownWriterThread();
        bool alreadyRunning();
        grpc::Status startWriterThread(grpc::ServerReaderWriter<claidservice::DataPackage, claidservice::DataPackage>* stream);
        grpc::Status processReading(grpc::ServerReaderWriter<claidservice::DataPackage, claidservice::DataPackage>* stream);

    private:
        void processWriting(grpc::ServerReaderWriter<claidservice::DataPackage, claidservice::DataPackage>* stream);
        void processPacket(claidservice::DataPackage& pkt, grpc::Status& status);
    
    private:
        SharedQueue<claidservice::DataPackage>& incomingQueue;
        SharedQueue<claidservice::DataPackage>& outgoingQueue;

        std::mutex writeThreadMutex; // protects the write thread
        std::unique_ptr<std::thread> writeThread;

        const std::string userToken;
        const std::string deviceID;

        friend class RemoteServiceImpl;
    };

}