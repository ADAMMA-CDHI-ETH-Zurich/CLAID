###########################################################################
# Copyright (C) 2023 ETH Zurich
# CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
# Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
# Centre for Digital Health Interventions (c4dhi.org)
# 
# Authors: Patrick Langer
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#         http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

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
