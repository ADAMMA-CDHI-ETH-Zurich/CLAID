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

#include "dispatch/core/capi.h"
#include "gtest/gtest.h"

#include "dispatch/core/CLAID.hh"

TEST(CAPITestSuite, BasicE2ETest) {
    const char* socket_path = "/tmp/capi_test_grpc.sock";
    const char* config_file = "dispatch/test/test_config.json";
    const char* host_id = "alex_client";
    const char* user_id = "user42";
    const char* device_id = "something_else";

    void* handle = start_core(socket_path, config_file, host_id, user_id, device_id);
    ASSERT_TRUE(handle != 0) << "Unable to start core middleware";
    shutdown_core(handle);
}
