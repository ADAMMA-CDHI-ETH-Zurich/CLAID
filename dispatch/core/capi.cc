#include <string>

#include "dispatch/core/middleware.hh"
#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/core/capi.h"


extern "C"
{

__attribute__((visibility("default"))) __attribute__((used))
void* start_core(const char* socket_path, const char* config_file, const char* host_id, const char* user_id, const char* device_id) {
    auto socketPath = std::string(socket_path);
    if (socketPath.find("unix://") != 0 && socketPath.find("localhost:") != 0) 
    {
        // Default to unix domain sockets
        socketPath = "unix://" + socketPath;
    }

    auto middleWare = new claid::MiddleWare(socketPath, config_file,
        host_id, user_id, device_id);

    auto status = middleWare->start();
    if (!status.ok()) {

        claid::Logger::logError("Failed to start middleware: %s", status.ToString().c_str());
        delete middleWare;
        return nullptr;
    }
    return static_cast<void*>(middleWare);
}

__attribute__((visibility("default"))) __attribute__((used))
void shutdown_core(void* handle) {
    if (handle) {
        auto middleWare = reinterpret_cast<claid::MiddleWare*>(handle);
        auto status = middleWare->shutdown();
        if (!status.ok()) 
        {
            // TODO: replace with proper logging.
            claid::Logger::printfln("Error shuting down middleware: %s", std::string(status.message()).c_str());
        }
        delete middleWare;
    }
}

__attribute__((visibility("default"))) __attribute__((used))
const char* get_socket_path(void* handle) {
    if (handle) {
        auto middleWare = reinterpret_cast<claid::MiddleWare*>(handle);
        
        // This is only safe as long as middleWare does not get deleted.
        return middleWare->getSocketPath().c_str();
    }
    claid::Logger::printfln("Cannot get socket path from middleware, handle is null.");
    return "";
}

}
