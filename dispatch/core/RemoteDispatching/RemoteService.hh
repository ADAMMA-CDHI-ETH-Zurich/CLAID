#pragma once

#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/RemoteDispatching/RemoteClientHandler.hh"

using claidservice::DataPackage;
using claidservice::RemoteClientInfo;

namespace claid
{
    typedef std::pair<std::string, std::string> RemoteClientKey;
    inline RemoteClientKey makeRemoteClientKey(const RemoteClientInfo& info)
    {
        return std::make_pair(info.user_token(), info.device_id());
    }

    class RemoteServiceImpl final : public claidservice::ClaidRemoteService::Service 
    {
    public:
        explicit RemoteServiceImpl();

        virtual ~RemoteServiceImpl() { };

        // This function is called by a RemoteDispatcherClient to after RegisterRemoteClient was called.
        // The function sets up the streaming between the Client and the Server. It first creates a writer thread
        // to send data to the Client via the stream, and then continuously reads packages from the stream from the RPC thread.
        // The function never returns as long as stream->Read() returns true. Hence, the function never returns as long as the
        // Client stays connected. If the client disconnects, the function automatically removes him from the list of Clients.
        grpc::Status SendReceivePackages(grpc::ServerContext* context,
            grpc::ServerReaderWriter<claidservice::DataPackage, claidservice::DataPackage>* stream) override;


        void shutdown();

    private:

        RemoteClientHandler* addRemoteClientHandler(const RemoteClientInfo& remoteClientInfo, grpc::Status& status);
        void stopAndRemoveRemoteClientHandler(const RemoteClientInfo& remoteClientInfo);

        grpc::Status getRemoteClientInfoFromHandshakePackage(const DataPackage& package, RemoteClientInfo& info);

    private:
        // claid::ModuleTable& moduleTable;
        std::map<RemoteClientKey, std::unique_ptr<RemoteClientHandler>> remoteClientHandlers;   
        std::mutex remoteClientHandlersMutex;    // protects remoteClientHandlers

    }; 

}
