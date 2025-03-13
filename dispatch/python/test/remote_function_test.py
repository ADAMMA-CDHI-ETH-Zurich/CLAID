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


import platform
import sys
import os

current_folder_path = os.path.dirname(os.path.abspath(__file__))
print(current_folder_path)

claid_path = os.path.join(current_folder_path, "..", "claid")
# Do not append, we need to prepend.
sys.path.insert(0,claid_path)

print("PATH IS ", claid_path)
from CLAID import CLAID
from module.module import Module
from module.module_factory import ModuleFactory
import asyncio
import os
import unittest

class TestModule(Module):
    def __init__(self):
        super().__init__()
        pass


    def initialize(self, properties):
       pass


class TestModule2(Module):
    def __init__(self):
        super().__init__()
        pass

    def initialize(self, properties):
        pass

module_factory = ModuleFactory()
module_factory.register_module(TestModule)
module_factory.register_module(TestModule2)
claid = CLAID()

print("Path localhost:1337")
path = "localhost:1337"
print("path ", path, file=sys.stderr)
asyncio.run(claid.start_async_with_custom_socket(path, "{}/dispatch/python/test/remote_function_test_config.json".format(os.getcwd()), "test_client", "user", "device", module_factory))

