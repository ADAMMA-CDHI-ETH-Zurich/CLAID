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