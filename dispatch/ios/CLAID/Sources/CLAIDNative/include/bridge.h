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

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void* start_core(const char* socket_path, const char* config_file, const char* host_id, const char* user_id, const char* device_id);
void* start_core_with_event_tracker(const char* socket_path, const char* config_file, const char* host_id, const char* user_id, const char* device_id, const char* common_data_path);
void shutdown_core(void* handle);

void* attach_cpp_runtime(void* handle);

#ifdef __cplusplus
}
#endif
