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

import unittest
import os
import time

from module.module_factory import ModuleFactory
from CLAID import CLAID

from module.module import Module
from logger.logger import Logger
global test_module1_started 
global test_module2_started 
global test_module3_started 
global test_module4_started 


test_module1_started = False
test_module2_started = False
test_module3_started = False
test_module4_started = False


class TestModule1(Module):
    def initialize(self, properties_map):
        global test_module1_started
        test_module1_data = properties_map.get("TestModule1Data")
        Logger.log_info("TestModule1 initialized")
        if test_module1_data == "42":
            test_module1_started = True
        else:
            print("TestModule1 did not find property TestModule1Data")

class TestModule2(Module):
    def initialize(self, properties_map):
        global test_module2_started
        test_module2_data = properties_map.get("TestModule2Data")
        if test_module2_data == "1337":
            test_module2_started = True
        else:
            print("TestModule2 did not find property TestModule2Data")

class TestModule3(Module):
    def initialize(self, properties_map):
        global test_module3_started
        test_module3_data = properties_map.get("TestModule3Data")
        print("TestModule 3 initialize")
        if test_module3_data == "420":
            test_module3_started = True
        else:
            print("TestModule3 did not find property TestModule3Data")

class TestModule4(Module):
    def initialize(self, properties_map):
        global test_module4_started
        test_module4_data = properties_map.get("TestModule4Data")
        if test_module4_data == "96":
            test_module4_started = True
        else:
            print("TestModule4 did not find property TestModule4Data")

# Simulating REGISTER_MODULE macros in Python



print("\n\n\n==== TEST START ====\n\n\n\n")
socket_path = "unix:///tmp/config_reload_test.sock"
config_file = "/Users/planger/Development/ModuleAPIV2/dispatch/test/config_reload_test_1.json"
host_id = "test_client"
user_id = "user42"
device_id = "something_else"

module_factory = ModuleFactory()
module_factory.register_module(TestModule1)
module_factory.register_module(TestModule2)
module_factory.register_module(TestModule3)
module_factory.register_module(TestModule4)

claid = CLAID()
result = claid.startCustomSocket(socket_path, config_file, host_id, user_id, device_id, module_factory)

time.sleep(2)


print("Loading new config")
claid.load_new_config_test("/Users/planger/Development/ModuleAPIV2/dispatch/test/config_reload_test_2.json")
time.sleep(0.2)
while(True):
    pass