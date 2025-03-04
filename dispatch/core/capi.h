/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

#ifndef CAPI_H_
#define CAPI_H_
#include <stdbool.h>  // for 'bool'

#ifdef __cplusplus
extern "C"
{
#endif

    void* start_core(const char* socket_path, const char* config_file, const char* host_id, const char* user_id, const char* device_id);
    void* start_core_with_event_tracker(const char* socket_path, const char* config_file, const char* host_id, const char* user_id, const char* device_id, const char* common_data_path);
    void shutdown_core(void* handle);
    
    void* attach_cpp_runtime(void* handle);

    bool load_new_config(void* handle, const char* config_file);

    void set_payload_data_path(void* handle, const char* path);
    const char* get_payload_data_path(void* handle);
    
    void set_common_data_path(void* handle, const char* path);
    const char* get_common_data_path(void* handle);

    const char* get_socket_path(void* handle);
    const char* get_host_id(void* handle);
    const char* get_user_id(void* handle);
    const char* get_device_id(void* handle);

    int get_log_sink_severity_level(void* handle);

    void enable_designer_mode(void* handle);
    void disable_designer_mode(void* handle);

#ifdef __cplusplus
}
#endif

#endif  // CAPI_H_
