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

# import unittest
# import os
# import time

# from module.module_factory import ModuleFactory
# from CLAID import CLAID

# from module.module import Module
# from logger.logger import Logger



# print("\n\n\n==== TEST START ====\n\n\n\n")
# socket_path = "unix:///tmp/config_reload_test.sock"
# config_file = "dispatch/python/test_config.json"
# host_id = "test_client"
# user_id = "user42"
# device_id = "something_else"

# module_factory = ModuleFactory()


# module_injection_storage_path = "/tmp/ModuleInjectionTest"
# claid = CLAID(module_injection_storage_path)
# result = claid.startCustomSocket(socket_path, config_file, host_id, user_id, device_id, module_factory)

# time.sleep(0.5)

# injected_module_content = ""
# with open("/home/lastchance/Desktop/ModuleAPIV2/bazel-ModuleAPIV2/dispatch/python/test/InjectedModule.py", "r") as file:
#     # Read the entire content of the file into a string
#     injected_module_content = file.read()

# new_modules = dict()
# new_modules["Test"] = (injected_module_content, ["MyInjectedModule"])
# claid.inject_new_modules(new_modules)
# claid.load_new_config_test("/home/lastchance/Desktop/ModuleAPIV2/dispatch/python/test/injection_config.json")

# while(True):
#     pass