from logger.logger import Logger
from datetime import datetime, timedelta
from module.scheduling import *
from module.scheduling.runnable_dispatcher import RunnableDispatcher

from abc import ABC, abstractmethod

class Module(ABC):
    def __init__(self):
        self.__id = None
        self.__is_initializing = False
        self.__is_initialized = False
        self.__is_terminating = False
        self.__subscriber_publisher = None
        self.__runnable_dispatcher = RunnableDispatcher()

    def module_fatal(self, error):
        errorMsg = f"Module \"{self.__id}\": {error}"
        Logger.log(SeverityLevel.FATAL, errorMsg)
        raise Exception(errorMsg)

    def module_error(self, error):
        errorMsg = f"Module \"{self.__id}\": {error}"
        Logger.log(SeverityLevel.ERROR, errorMsg)

    def module_warning(self, warning):
        warningMsg = f"Module \"{self.__id}\": {warning}"
        Logger.log(SeverityLevel.WARNING, warningMsg)


    def start(self, subscriber_publisher, properties):
        if self.__is_initialized:
            self.module_error("Initialize called twice!")
            return False

        self.__subscriber_publisher = subscriber_publisher

        if not self.__runnable_dispatcher.start():
            self.module_error("Failed to start RunnableDispatcher.")
            return False

        self.__is_initializing = True
        self.__is_initialized = False

        self.initialize_internal(properties)

        self.__is_initializing = False
        self.__subscriber_publisher = None
        return True

    def initialize_internal(self, properties):
        self.initialize(properties)
        self.__is_initialized = True

    @abstractmethod
    def initialize(self, properties):
        pass

    def set_id(self, module_id):
        self.__id = module_id

    def get_id(self):
        return self.__id

    def enqueue_runnable(self, runnable):
        self.__runnable_dispatcher.addRunnable(runnable)

    def register_periodic_function(self, name, callback, interval):
        self.register_periodic_function_with_start_time(name, callback, interval, Time.now() + interval)

    def register_periodic_function_with_start_time(self, name, callback, interval, start_time):
        if interval.total_seconds() == 0:
            self.module_error(f"Error in registerPeriodicFunction: Cannot register periodic function \"{name}\" with a period of 0 seconds.")

        if name in self.timers:
            self.module_error(f"Tried to register function with name \"{name}\", but a periodic function with the same name was already registered before.")

        function_runnable = FunctionRunnable(callback)
        runnable = ScheduledRunnable(
            runnable=function_runnable,
            schedule=ScheduleRepeatedIntervall(start_time, interval))

        self.timers[name] = runnable
        self.__runnable_dispatcher.addRunnable(runnable)
        Logger.printfln("Registered periodic runnable %s", name)

    def register_scheduled_function(self, name, start_time, function):
        if start_time < Time.now():
            self.module_warning(f"Failed to schedule function \"{name}\" at time {start_time.strftime('%d.%m.%y - %H:%M:%S')}. "
                               f"The time is in the past. It is now: {Time.now().strftime('%d.%m.%y - %H:%M:%S')}")

        function_runnable = FunctionRunnable(function)
        runnable = ScheduledRunnable(
            runnable=function_runnable,
            schedule=ScheduleOnce(start_time))

        self.timers[name] = runnable
        self.__runnable_dispatcher.addRunnable(runnable)

    def unregister_periodic_function(self, name):
        if name not in self.timers:
            self.module_error(f"Error, tried to unregister periodic function \"{name}\" but function was not found in the list of registered timers. "
                             f"Was a function with this name ever registered before?")
        self.timers[name].runnable.invalidate()
        del self.timers[name]

    def unregister_all_periodic_functions(self):
        for entry in self.timers.values():
            entry.runnable.invalidate()
        self.timers.clear()

    def shutdown(self):
        self.__is_terminating = True

        self.terminate_internal()

        Logger.logInfo("Runnable dispatcher stop 1")
        self.__runnable_dispatcher.stop()
        Logger.logInfo("Runnable dispatcher stop 2")
        self.__is_initialized = False
        Logger.logInfo("Runnable dispatcher stop 3")

    def terminate_internal(self):
        self.terminate()
        self.unregister_all_periodic_functions()
        self.__is_terminating = False

    def terminate(self):
        pass

    def on_connected_to_remote_server(self):
        pass

    def on_disconnected_from_remote_server(self):
        pass

    def notify_connected_to_remote_server(self):
        self.on_connected_to_remote_server()

    def notify_disconnected_from_remote_server(self):
        self.on_disconnected_from_remote_server()