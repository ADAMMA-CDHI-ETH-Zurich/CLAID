import importlib
from logger.logger import Logger
import pathlib
import os
import copy

import sys
# Helper class for the module_manager
# Allows to inject new Modules at runtime by importing or updating (new) Python files.
class ModuleInjector():
    def __init__(self, storage_path_for_injected_modules, module_factory):
        self.storage_path_for_injected_modules = storage_path_for_injected_modules
        self.injected_modules = dict() # stores name of injected python files, their lib handles and the module names.
        self.original_module_factory = copy.copy(module_factory)
        sys.path.append(storage_path_for_injected_modules)

    
    # python_file_path is the path to a python file.
    # module_names is a list of CLAID module names (i.e., class names) which can be found in the python file (i.e., 
    # they need to be defined in the raw_python_code). They will be added to the CLAID ModuleFactory.
    # If you call this function again with the same name, then the previous code will be overriden and the 
    # python code will be updated using importlib.reload.
    # It is currently NOT possible to unload modules.
    def inject_claid_modules_from_python_file(self, name: str, module_names : list):
        python_handle = None
        try:
            file_path = os.path.join(self.storage_path_for_injected_modules, name)
            file_path += ".py"

            with open(file_path, 'w') as file:
                # Write content to the file
                file.write(raw_python_code)

            if name in self.injected_modules:
                Logger.log_debug(f"ModuleInjector: Python code {name} already exists, updating lib.")
                python_handle = importlib.reload(name)
            else:
                Logger.log_debug(f"ModuleInjector: Python code {name} not injected before, importing module")
                python_handle = importlib.import_module(name)
            self.injected_modules[name] = (python_handle, module_names)
            return True
        except Exception as e:
            Logger.log_error(f"Failed to inject new Python Module from code \"{name}\":  {e}")
            return False
        
                


    def rebuild_get_module_factory(self):

        new_module_factory = copy.copy(self.original_module_factory)

        print(self.injected_modules)
        for code_name in self.injected_modules:
            print(self.injected_modules[code_name])
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
                    
                new_module_factory.register_module(clz)
                Logger.log_debug(f"Registered module {module_name} of python code {code_name}")
        
        self.original_module_factory = new_module_factory
        return self.original_module_factory
