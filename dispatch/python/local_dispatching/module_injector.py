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

import importlib
from logger.logger import Logger
import pathlib
import os
import copy

import inspect

import sys

from claid.module import Module

# Helper class for the module_manager
# Allows to inject new Modules at runtime by importing or updating (new) Python files.
class ModuleInjector():
    def __init__(self):
        self.injected_modules = dict() # stores name of injected python files, their lib handles and the module names.

    
    # base path is the path of where the python files are located (typically this is the payloadDataPath of the middleware)
    # python_file_path is the path to a python file.
    # module_names is a list of CLAID module names (i.e., class names) which can be found in the python file (i.e., 
    # they need to be defined in the raw_python_code). They will be added to the CLAID ModuleFactory.
    # If you call this function again with the same name, then the previous code will be overriden and the 
    # python code will be updated using importlib.reload.
    # It is currently NOT possible to unload modules.
    def inject_claid_modules_from_python_file(self, base_path: str, python_module_name: str, claid_module_names : list):
        
        
        python_handle = None
        try:

            python_module_name = python_module_name.replace(".py", "")

            python_handle = self.__load_get_python_module(base_path, python_module_name)

            self.injected_modules[python_module_name] = (python_handle, claid_module_names)
            
            return True
        except Exception as e:
            Logger.log_error(f"Failed to inject new CLAID Module from Python Module/Code \"{python_module_name}\":  {e}")
            return False
        
    def __load_get_python_module(self, base_path, module_name: str):
        sys.path.append(base_path)
        if module_name in self.injected_modules:
            Logger.log_debug(f"ModuleInjector: Python code {base_path}/{module_name} already exists, updating lib.")
            python_handle = importlib.reload(module_name)
            return python_handle
        else:
            Logger.log_debug(f"ModuleInjector: Python code {base_path}/{module_name} not injected before, importing module")
            python_handle = importlib.import_module(module_name)
            return python_handle


        
    def inject_claid_modules_from_all_files_in_folder(self, folder_path : str):
        
        files = [f for f in os.listdir(folder_path) if f.endswith(".py")]

        
        found_modules = list()
        for file in files:
            python_module_path = os.path.join(folder_path, file.replace(".py", ""))

            if python_module_path not in found_modules:
                found_modules.append(python_module_path)

        for python_module_path in found_modules:

            Logger.log_debug("Loading Python file {}".format(python_module_path))

            base_path = os.path.dirname(python_module_path)
            module_name = os.path.basename(python_module_path)
            python_handle = self.__load_get_python_module(base_path, module_name)

            all_classes = inspect.getmembers(python_handle, inspect.isclass)

            # Filter classes based on inheritance
            claid_module_class_names = [name for name, cls in all_classes if issubclass(cls, Module) and cls != Module]

            Logger.log_debug("Found classes {} in Python file {}".format(claid_module_class_names, python_module_path))

            self.injected_modules[python_module_path] = (python_handle, claid_module_class_names)
            


    def get_injected_module_classes(self):


        module_classes = dict()
        for code_name in self.injected_modules:
            python_handle, module_names = self.injected_modules[code_name]
            if python_handle is None:
                continue


            for module_name in module_names:
                if not hasattr(python_handle, module_name):
                    Logger.log_warning(f"Failed to inject CLAID Module \"{module_name}\" from code \"{code_name}\"." + \
                                    "Module definition (class) was not found inside the code.")
                    continue

                clz = getattr(python_handle, module_name)

                if not callable(clz):
                    Logger.log_warning(f"Failed to inject CLAID Module \"{module_name}\" from code \"{code_name}\"." + \
                                    "Module definition was found, but is not callable (i.e., is neither class nor function).")
                    
                module_classes[module_name] = clz
                Logger.log_debug(f"Registered module {module_name} of python code {code_name}")
        
        return module_classes
