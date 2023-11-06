#include <string>

#include "dispatch/core/middleware.hh"
#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/core/capi.h"

extern "C"
{

__attribute__((visibility("default"))) __attribute__((used))
void* start_core(const char* socket_path, const char* config_file, const char* host_id, const char* user_id, const char* device_id) {
    auto socketPath = std::string(socket_path);
    if (socketPath.find("unix://") != 0) {
        socketPath = "unix://" + socketPath;
    }

    auto middleWare = new claid::MiddleWare(socketPath, config_file,
        host_id, user_id, device_id);

    auto status = middleWare->start();
    if (!status.ok()) {
        std::cout << "Failed to start middleware: " << status << "\n";
        delete middleWare;
        return nullptr;
    }
    return static_cast<void*>(middleWare);
}

__attribute__((visibility("default"))) __attribute__((used))
void shutdown_core(void* handle) {
    if (handle) {
        auto middleWare = reinterpret_cast<claid::MiddleWare*>(handle);
        claid::Logger::printfln("Shutting down middleware");
        auto status = middleWare->shutdown();
        if (!status.ok()) {
            // TODO: replace with proper logging.
            claid::Logger::printfln("Error shuting down middleware: %s", std::string(status.message()).c_str());
        }
        delete middleWare;
    }
}

}
