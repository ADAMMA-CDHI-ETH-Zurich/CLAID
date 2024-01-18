#ifndef CAPI_H_
#define CAPI_H_


#ifdef __cplusplus
extern "C"
{
#endif

    void* start_core(const char* socket_path, const char* config_file, const char* host_id, const char* user_id, const char* device_id);
    void shutdown_core(void* handle);
    const char* get_socket_path(void* handle);

    void* attach_cpp_runtime(void* handle);

    bool load_new_config(void* handle, const char* config_file);

#ifdef __cplusplus
}
#endif

#endif  // CAPI_H_
