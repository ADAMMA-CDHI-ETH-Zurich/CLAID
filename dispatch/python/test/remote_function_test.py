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
import asyncio
import unittest
import time
import threading

current_folder_path = os.path.dirname(os.path.abspath(__file__))
print(current_folder_path)

claid_path = os.path.join(current_folder_path, "..")
sys.path.insert(0, claid_path)

claid_path = os.path.join(current_folder_path, "..", "claid")
sys.path.insert(0, claid_path)

print("PATH IS ", claid_path)
from claid.CLAID import CLAID
from claid.module.module import Module
from claid.module.module_factory import ModuleFactory
from datetime import datetime

# Global variables to track module initialization
module1_started = False
module2_started = False
module1_function_sent = False
module2_function_called = False
module1_function_returned = False
module1_function_return_correct = False

class TestModule(Module):
    def __init__(self):
        super().__init__()

    async def initialize(self, properties):
        global module1_started
        self.function = self.map_remote_function_of_module("Module2", "test_function", int(0), str(""))
        module1_started = True
        print("TestModule1 initialized.")
        await self.register_scheduled_function("CallFunction", self.call_function, datetime.now())

    async def call_function(self):
        print("Calling function")
        global module1_function_sent
        module1_function_sent = True
        future = self.function.execute("Test")
        print("Called function awaiting future")
        future.then(self.on_result)


    async def on_result(self,data):
        print("Got result: ", data)
        global module1_function_returned
        global module1_function_return_correct
        module1_function_returned = True
        if data == 42:
            module1_function_return_correct = True

class TestModule2(Module):
    def __init__(self):
        super().__init__()

    async def initialize(self, properties):
        global module2_started
        module2_started = True
        self.register_remote_function("test_function", self.test_function, int(0), str(""))
        print("TestModule2 initialized.")

    async def test_function(self, value: str):
        global module2_function_called 
        module2_function_called = True
        print("TestModule2 says: ", value)
        return 42

# Register modules with the factory
module_factory = ModuleFactory()
module_factory.register_module(TestModule)
module_factory.register_module(TestModule2)

claid = CLAID()

path = "localhost:1337"
#config_path = "{}/dispatch/python/test/remote_function_test_config.json".format(os.getcwd())
config_path = "{}/remote_function_test_config.json".format(os.getcwd())


# Function to start CLAID asynchronously
async def start_claid():
    await claid.start_async_with_custom_socket(
        path, config_path, 
        "test_client", "user", "device", module_factory
    )

# Run CLAID in a separate thread to avoid blocking
def run_claid_in_background():
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    loop.run_until_complete(start_claid())

threading.Thread(target=run_claid_in_background, daemon=True).start()

class ModuleInitializationTest(unittest.TestCase):
    def test_modules_started(self):
        """ Waits 10 seconds and checks if modules have started, then exits. """
        global module1_started, module2_started, test_completed

        # Wait for 10 seconds
        for i in range(10):
            if module1_started and module2_started:
                break
            time.sleep(1)  # Wait 1 second before checking again

        # Assert both modules have started
        self.assertTrue(module1_started, "TestModule1 did not start within 10 seconds.")
        self.assertTrue(module2_started, "TestModule2 did not start within 10 seconds.")
        time.sleep(10)

        self.assertTrue(module1_function_sent, "TestModule1 did not call remote function.")
        self.assertTrue(module2_function_called, "TestModule1 did not receive remote function call.")
        self.assertTrue(module1_function_returned, "TestModule1 did not receive response of remote function.")
        self.assertTrue(module1_function_return_correct, "TestModule1 did receive response from remote function but return value was incorrect.")



        # Mark test as completed
        test_completed = True
        print("Test completed successfully.")

if __name__ == "__main__":
    unittest.main()
