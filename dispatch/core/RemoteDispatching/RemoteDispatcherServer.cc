#include "dispatch/core/RemoteDispatching/RemoteDispatcherServer.hh"

#include <grpcpp/server_builder.h>
#include <grpcpp/create_channel.h>

#include <google/protobuf/text_format.h>



namespace claid 
{
    RemoteDispatcherServer::RemoteDispatcherServer(const std::string& addr)
        : addr(addr)
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
        server->Shutdown();
        server->Wait();
    }

    void RemoteDispatcherServer::buildAndStartServer()
    {
        grpc::ServerBuilder builder;
        builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
        builder.RegisterService(&remoteServiceImpl);
        server = builder.BuildAndStart();
    }
}