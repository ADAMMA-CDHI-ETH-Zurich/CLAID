from module.scheduling.runnable import Runnable

class FunctionRunnable(Runnable):
    def __init__(self, function):
        super().__init__()
        self.function = function

    def run(self):
        self.function()