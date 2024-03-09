#include <string>

#include "dispatch/core/middleware.hh"
#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/core/capi.h"

#include "dispatch/core/CLAID.hh"

#include "dispatch/proto/sensor_data_types.pb.h"
#include "dispatch/proto/claidservice.pb.h"


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
    claid::Logger::logInfo("CLAID middleware started, returning handle %u %s", middleWare, middleWare->getSocketPath().c_str());
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
            claid::Logger::logWarning("Error shuting down middleware: %s", std::string(status.message()).c_str());
        }
        delete middleWare;
    }
}

__attribute__((visibility("default"))) __attribute__((used))
void* attach_cpp_runtime(void* handle)
{
    claid::Logger::logInfo("Attach C++ runtime called %u", handle);
    claid::CLAID* claid = new claid::CLAID();
    if(!claid->attachCppRuntime(handle))
    {
        claid::Logger::logError("CLAID capi failed to attach_cpp_runtime.");
        delete claid;
        return nullptr;
    }
    claid::Logger::logInfo("Successfully attached C++ runtime to handle %u", handle);
    
    return claid;
}

__attribute__((visibility("default"))) __attribute__((used))
bool load_new_config(void* handle, const char* config_file)
{
    claid::Logger::logInfo("capi load_new_config called");
    if (handle) {
        auto middleWare = reinterpret_cast<claid::MiddleWare*>(handle);
        
        // This is only safe as long as middleWare does not get deleted.
        absl::Status status = middleWare->loadNewConfig(config_file);
        if(!status.ok())
        {
            claid::Logger::logError("Failed to load new config: %s", status.ToString().c_str());
            return false;
        }
        return true;
    }
    claid::Logger::logError("Cannot get socket path from middleware, handle is null.");
    return false;
}

__attribute__((visibility("default"))) __attribute__((used))
void hello_world()
{
    printf("Hello world from the CLAID framework!\n");
}

__attribute__((visibility("default"))) __attribute__((used))
void set_payload_data_path(void* handle, const char* path)
{
    if(!handle)
    {
        claid::Logger::logError("Cannot set payload data path, handle is null.");
        return;
    }
    std::string payloadPath(path);

    auto middleWare = reinterpret_cast<claid::MiddleWare*>(handle);
    middleWare->setPayloadDataPath(payloadPath);
}

__attribute__((visibility("default"))) __attribute__((used))
const char* get_payload_data_path(void* handle)
{
    if(!handle)
    {
        claid::Logger::logError("Cannot set payload data path, handle is null.");
        return "";
    }

    auto middleWare = reinterpret_cast<claid::MiddleWare*>(handle);
    return middleWare->getPayloadDataPath().c_str();
}

__attribute__((visibility("default"))) __attribute__((used))
const char* get_socket_path(void* handle) {
    if (handle) {
        auto middleWare = reinterpret_cast<claid::MiddleWare*>(handle);
        
        // This is only safe as long as middleWare does not get deleted.
        return middleWare->getSocketPath().c_str();
    }
    claid::Logger::logError("Cannot get socket path from middleware, handle is null.");
    return "";
}

__attribute__((visibility("default"))) __attribute__((used))
const char* get_host_id(void* handle) {
    if (handle) {
        auto middleWare = reinterpret_cast<claid::MiddleWare*>(handle);
        
        // This is only safe as long as middleWare does not get deleted.
        return middleWare->getHostId().c_str();
    }
    claid::Logger::logError("Cannot get hostId from middleware, handle is null.");
    return "";
}

__attribute__((visibility("default"))) __attribute__((used))
const char* get_user_id(void* handle) {
    if (handle) {
        auto middleWare = reinterpret_cast<claid::MiddleWare*>(handle);
        
        // This is only safe as long as middleWare does not get deleted.
        return middleWare->getUserId().c_str();
    }
    claid::Logger::logError("Cannot get userId from middleware, handle is null.");
    return "";
}

__attribute__((visibility("default"))) __attribute__((used))
const char* get_device_id(void* handle) {
    if (handle) {
        auto middleWare = reinterpret_cast<claid::MiddleWare*>(handle);
        
        // This is only safe as long as middleWare does not get deleted.
        return middleWare->getDeviceId().c_str();
    }
    claid::Logger::logError("Cannot get deviceId from middleware, handle is null.");
    return "";
}


__attribute__((visibility("default"))) __attribute__((used))
void enable_designer_mode(void* handle)
{
    if(!handle)
    {
        claid::Logger::logError("Cannot enable designer model, handle is null.");
        return;
    }
    claid::Logger::logInfo("capi enable designer mode called");

    auto middleWare = reinterpret_cast<claid::MiddleWare*>(handle);
    middleWare->enableDesignerMode();
}

__attribute__((visibility("default"))) __attribute__((used))
void disable_designer_mode(void* handle)
{
    if(!handle)
    {
        claid::Logger::logError("Cannot enable designer model, handle is null.");
        return;
    }

    auto middleWare = reinterpret_cast<claid::MiddleWare*>(handle);
    middleWare->disableDesignerMode();
}

int get_log_sink_severity_level(void* handle)
{
    if(!handle)
    {
        claid::Logger::logError("Cannot get log sink severity level, handle is null.");
        return 0;
    }

    auto middleWare = reinterpret_cast<claid::MiddleWare*>(handle);
    return middleWare->getLogSinkSeverityLevel();
}

// Make sure all sensor data types are included.
// URGENT TODO: Fix bazel not linking in all protobuf types from a cc_proto_library.
// Possibly related to https://github.com/bazelbuild/bazel/issues/18459

// Instantiate AudioData
claid::AudioData audioData;

// Instantiate AccelerationUnitValue
claid::AccelerationUnitValue accelerationUnitValue;

// Instantiate ActivityName
claid::ActivityName activityName;

// Instantiate AccelerationSample
claid::AccelerationSample accelerationSample;

// Instantiate AccelerationData
claid::AccelerationData accelerationData;

// Instantiate GyroscopeSample
claid::GyroscopeSample gyroscopeSample;

// Instantiate GyroscopeData
claid::GyroscopeData gyroscopeData;

// Instantiate HeartRateSample
claid::HeartRateSample heartRateSample;

// Instantiate HeartRateData
claid::HeartRateData heartRateData;

// Instantiate BatterySample
claid::BatterySample batterySample;

// Instantiate BatteryData
claid::BatteryData batteryData;
}
