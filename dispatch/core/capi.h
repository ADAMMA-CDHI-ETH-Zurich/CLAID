#ifndef CAPI_H_
#define CAPI_H_

#ifdef __cplusplus
extern "C"
{
#endif

    void* start_core(const char* socket_path, const char* config_file, const char* host_id, const char* user_id, const char* device_id);
    void shutdown_core(void* handle);

#ifdef __cplusplus
}
#endif

#endif  // CAPI_H_
