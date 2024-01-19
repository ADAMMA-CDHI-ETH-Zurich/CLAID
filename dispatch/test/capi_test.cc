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
