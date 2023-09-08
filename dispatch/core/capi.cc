#include <string>

#include "dispatch/core/capi.h"
#include "dispatch/core/middleware.hh"

void* start_core(const char* socket_path, const char* config_file, const char* user_id, const char* device_id) {
    auto middleWare = new claid::MiddleWare(socket_path, config_file,
        "this_hostname", user_id, device_id);

    auto status = middleWare->start();
    if (!status.ok()) {
        delete middleWare;
        return nullptr;
    }
    return middleWare;
}

void shutdown_core(void* handle) {
    if (handle) {
        auto middleWare = reinterpret_cast<claid::MiddleWare*>(handle);
        auto status = middleWare->shutdown();
        if (!status.ok()) {
            // TODO: replace with proper logging.
            std::cout << "Error shutind down middleware" << std::endl;
        }
        delete middleWare;
    }
}

