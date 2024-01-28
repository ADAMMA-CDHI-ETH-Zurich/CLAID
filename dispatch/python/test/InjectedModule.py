from module.module import Module

class MyInjectedModule(Module):
    def __init__(self):
        super().__init__()
        pass

    def initialize(self, properties):
        print("MyInjectedModule started!")
        return super().initialize(properties)