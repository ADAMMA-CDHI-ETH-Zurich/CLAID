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
      RemoteDispatcherServer(const std::string& addr);
      virtual ~RemoteDispatcherServer() {};

      absl::Status start();
      void shutdown();

    private:
      void buildAndStartServer();

    private:
      const std::string addr;
      HostUserTable& hostUserTable;

      RemoteServiceImpl remoteServiceImpl;
      std::unique_ptr<grpc::Server> server;
  };

}
