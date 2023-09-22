#include "dispatch/core/RemoteDispatching/RemoteDispatcherServer.hh"

#include <grpcpp/server_builder.h>
#include <grpcpp/create_channel.h>

#include <google/protobuf/text_format.h>



namespace claid 
{
    RemoteDispatcherServer::RemoteDispatcherServer(const std::string& addr, HostUserTable& hostUserTable)
        : addr(addr), hostUserTable(hostUserTable), remoteServiceImpl(hostUserTable)
    {

    }

    absl::Status RemoteDispatcherServer::start() 
    {
        buildAndStartServer();
        if (!server) {
            return absl::InvalidArgumentError(
                absl::StrCat("Failed to start RemoteDispatcherServer with address \"", this->addr, "\"."));
        }
        return absl::OkStatus();
    }

    void RemoteDispatcherServer::shutdown() 
    {
        std::cout << "RemoteDispatcherServer shutdown 1\n";
        // server->Shutdown() will hang indefintely as long as there are still ongoing RPC calls by any client.
        // The clients call SendReceivePackages to stream data to/from the server. The SendReceivePackage RPC call
        // typically never returns, as long as the client is alive.
        // Hence, we have to forcefully end all client's RPC calls registered in the RemoteService.
        this->remoteServiceImpl.shutdown();
        // Now we can safely call server->Shutdown();
        server->Shutdown();
        std::cout << "RemoteDispatcherServer shutdown 2\n";
        server->Wait();
        std::cout << "RemoteDispatcherServer shutdown 3\n";
    }

    void RemoteDispatcherServer::buildAndStartServer()
    {
        grpc::ServerBuilder builder;
        builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
        builder.RegisterService(&remoteServiceImpl);
        server = builder.BuildAndStart();
    }
}