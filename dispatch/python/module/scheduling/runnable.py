from abc import ABC, abstractmethod

class Runnable(ABC):
    def __init__(self):
        super().__init__()
        self.valid = True
        self.exception_thrown = False
        self.exception_message = ""
        self.was_executed = False
        self.catch_exceptions = False
        self.stop_dispatcher_after_this_runnable = False

    @abstractmethod
    def run(self):
        pass

    def invalidate(self):
        self.valid = False

    def is_valid(self):
        return self.valid

    def was_exception_thrown(self):
        return self.exception_thrown

    def get_exception_message(self):
        return self.exception_message

    def set_exception(self, exception_message):
        self.exception_message = exception_message
        self.exception_thrown = True
