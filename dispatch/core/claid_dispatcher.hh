#ifndef CLAID_DISPATCHER_HH_
#define CLAID_DISPATCHER_HH_

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include "dispatch/proto/claidservice.grpc.pb.h"

using namespace std;

using grpc::Status;
using grpc::ServerContext;
using grpc::ServerReaderWriter;

using claidservice::ClaidService;
using claidservice::DataPackage;
using claidservice::ModuleListRequest;
using claidservice::ModuleListResponse;

namespace claid {

class ClaidDispatcher {
  public:
    ClaidDispatcher(const std::string& socketPath, const std::string& configFile);
    virtual ~ClaidDispatcher() {};

};

class ClaidServiceImpl final : public claidservice::ClaidService::Service {
  public:
    explicit ClaidServiceImpl(const string& socketPath);
    virtual ~ClaidServiceImpl() {}; 
    Status GetModuleList(ServerContext* context, 
        const ModuleListRequest* req, 
        ModuleListResponse* resp) override;

    Status SendReceivePackages(ServerContext* context,
        ServerReaderWriter<DataPackage, DataPackage>* stream) override; 

}; // class ClaidServiceImpl 

}  // namespace claid 

#endif  // CLAID_DISPATCHER_H_
