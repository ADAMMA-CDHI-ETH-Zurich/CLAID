#include "dispatch/core/claid_dispatcher.hh"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using std::chrono::system_clock;

using namespace claid;

ClaidDispatcher::ClaidDispatcher(const string& socketPath, const string& configFile) {
    // Create an instance of the server implmentation

    // 
}

Status ClaidServiceImpl::GetModuleList(ServerContext* context, 
    const ModuleListRequest* req, 
    ModuleListResponse* resp) {

}

Status ClaidServiceImpl::SendReceivePackages(ServerContext* context,
    ServerReaderWriter<DataPackage, DataPackage>* stream) {

}

