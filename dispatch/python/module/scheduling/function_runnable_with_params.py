import types

from module.scheduling.runnable import Runnable

class FunctionRunnableWithParams(Runnable):
    def __init__(self, function=None):
        super().__init__()
        self.function = function
        self.stack = tuple()

    def run(self):

        self.function(*self.stack)

    def set_params(self, *params):
        self.stack = params

    def bind(self, func, object):
        if not callable(func):
            raise ValueError("The second argument must be a callable function/method.")
    
        if not hasattr(obj, func.__name__):
            raise AttributeError(f"The object does not have a method named {func.__name__}.")

        self.function = types.MethodType(func, object)

    def bind_with_params(self, func, obj, *args):
        self.bind(func, obj)
        self.set_params(*args)