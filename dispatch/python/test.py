import ctypes
import sys
import pathlib
import sys  # Import the sys module to access system-specific parameters and functions

# Print the Python version to the console
print("Python version")

# Use the sys.version attribute to get the Python version and print it
print(sys.version)

exit(0)
# module_factory = ModuleFactory()
# claid = CLAID()
# claid.start("/Users/planger/Development/ModuleAPIV2/dispatch/test/data_saver_json_test.json", "test_client", "user", "device", module_factory)
socket = "unix:///tmp/claid_socket.grpc".encode('utf-8')
config = "/Users/planger/Development/ModuleAPIV2/dispatch/test/remote_dispatching_test.json".encode('utf-8')
client = "test_client".encode('utf-8')
libname = "/Users/planger/Development/ModuleAPIV2/bazel-bin/dispatch/core/libclaid_capi.dylib"
lib = ctypes.CDLL(libname)
# const char* socket_path, const char* config_file, const char* host_id, const char* user_id, const char* device_id
lib.start_core(socket, config, client, "test", "test", "test")

# mutator = TypeMapping.get_mutator(value)

# value = {"test": 42, "test2" : 33}
# mutator.set_package_payload(package, value)
# print(package)
# print(mutator.get_package_payload(package))


# import pathlib
