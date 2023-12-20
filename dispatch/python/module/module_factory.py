from module.module import Module
from logger.logger import Logger 

class ModuleFactory:
    def __init__(self):
        self.registered_module_classes = {}

    def register_module(self, clz):
        class_name = clz.__name__

        if not issubclass(clz, Module):
            raise Exception("Cannot register class \"{}\" to ModuleFactory. Class is not derived from Module.".format(class_name))

        self.registered_module_classes[class_name] = clz
        print(f"Register class {class_name}")
        return True

    def get_instance(self, class_name, module_id):
        if class_name in self.registered_module_classes:
            try:
                module = self.registered_module_classes[class_name]()
                module.set_id(module_id)
                return module
            except Exception as e:
                Logger.log_error(f"Failed to instantiate module \"{module_id}\" of class \"{class_name}\".\n"
                f"Got error: {e}")
                return None
        Logger.log_error(f"Failed to instantiate module \"{module_id}\" of class \"{class_name}\". Class was not registered.\n"
        f"Registered classes are: {self.registered_module_classes}")
        return None

    def is_module_class_registered(self, module_class):
        return module_class in self.registered_module_classes

    def get_registered_module_classes(self):
        return list(self.registered_module_classes.keys())