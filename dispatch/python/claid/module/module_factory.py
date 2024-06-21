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

from claid.module.module import Module
from logger.logger import Logger 

from local_dispatching.module_injector import ModuleInjector

class ModuleFactory:
    def __init__(self):
        self.registered_module_classes = {}

        # Concatenation of registered and injected modules
        self.all_available_module_classes = {} 

        

        self.__module_injector = ModuleInjector()

    def register_module(self, clz):
        class_name = clz.__name__

        if not issubclass(clz, Module):
            raise Exception("Cannot register class \"{}\" to ModuleFactory. Class is not derived from Module.".format(class_name))

        self.registered_module_classes[class_name] = clz
        self.all_available_module_classes[class_name] = clz
        print(f"Register class {class_name}")
        return True
    
    # Scans all Python files in the specified directory,
    # loads them, searches them for classes inheriting from claid.module.Module and
    # adds the Modules to the ModuleFactory.
    def register_all_modules_found_in_path(self, path: str):
        self.__module_injector.inject_claid_modules_from_all_files_in_folder(path)

        self.all_available_module_classes.clear()
        self.all_available_module_classes.update(self.registered_module_classes)
        self.all_available_module_classes.update(self.__module_injector.get_injected_module_classes())
        Logger.log_info("Available instances from path " + str(self.all_available_module_classes))

        
    def inject_claid_modules_from_python_file(self, base_path: str, python_module_name: str, claid_module_names : list):
        self.__module_injector.inject_claid_modules_from_python_file(base_path, python_module_name, claid_module_names)

        self.all_available_module_classes.clear()
        self.all_available_module_classes.update(self.registered_module_classes)
        self.all_available_module_classes.update(self.__module_injector.get_injected_module_classes())
        Logger.log_info("Available instances from file " + str(self.all_available_module_classes))


    def get_instance(self, class_name, module_id):
        Logger.log_info("Available instances " + str(self.all_available_module_classes))
        if class_name in self.all_available_module_classes:
            try:
                module = self.all_available_module_classes[class_name]()
                module.set_id(module_id)
                return module
            except Exception as e:
                Logger.log_error(f"Failed to instantiate module \"{module_id}\" of class \"{class_name}\".\n"
                f"Got error: {e}")
                return None
        Logger.log_error(f"Failed to instantiate module \"{module_id}\" of class \"{class_name}\". Class was not registered.\n"
        f"Registered classes are: {self.all_available_module_classes}")
        return None
    
    def get_module_annotation(self, class_name, module_annotator):

        if not class_name in self.all_available_module_classes:
            return False 
        else:
            class_type = self.all_available_module_classes[class_name]
            annotate_module_function = "annotate_module"
            if(hasattr(class_type, annotate_module_function) and callable(getattr(class_type, annotate_module_function))):
                getattr(class_type, annotate_module_function)(module_annotator)
                return True
            else:
                return False


    def is_module_class_registered(self, module_class):
        return module_class in self.all_available_module_classes

    def get_registered_module_classes(self):
        Logger.log_info("Available instances " + str(self.all_available_module_classes))
        return list(self.all_available_module_classes.keys())

    def register_default_modules(self):

        from data_collection.plot.acceleration_view import AccelerationView
        from data_collection.plot.gyroscope_view import GyroscopeView
        from data_collection.plot.heartrate_view import HeartRateView
        self.register_module(AccelerationView)
        self.register_module(GyroscopeView)
        self.register_module(HeartRateView)
    
    def print_registered_modules(self):
        Logger.log_info("Registered Module classes: ")
        for key in self.all_available_module_classes:
            Logger.log_info(key)

    def get_path_to_python_file_of_module(self, module_type):

        if not self.is_module_class_registered(module_type):
            return None
        
        import inspect
        clz = self.all_available_module_classes[module_type]
        return inspect.getfile(clz)