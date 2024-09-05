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

from logger.logger import Logger
from datetime import datetime, timedelta
from module.scheduling.scheduled_runnable import ScheduledRunnable
from module.scheduling.runnable_dispatcher import RunnableDispatcher
from module.scheduling.function_runnable import FunctionRunnable
from module.scheduling.schedule_repeated_intervall import ScheduleRepeatedIntervall
from module.channel import Channel
from module.subscriber import Subscriber

from abc import ABC, abstractmethod

from datetime import datetime
from module.scheduling.function_runnable_with_params import FunctionRunnableWithParams
from module.scheduling.scheduled_runnable import ScheduledRunnable
from module.scheduling.schedule_once import ScheduleOnce
from dispatch.proto.claidservice_pb2 import LogMessageSeverityLevel, LogMessage, LogMessageEntityType, Runtime

import time

class Module(ABC):
    def __init__(self):
        self.__id = None
        self.__is_initializing = False
        self.__is_initialized = False
        self.__is_terminating = False
        self.__subscriber_publisher = None
        self.__timers = dict()


    def module_fatal(self, error):
        errorMsg = f"Module \"{self.__id}\": {error}"
        Logger.log(LogMessageSeverityLevel.FATAL, errorMsg, LogMessageEntityType.MODULE, self.__id)
        raise Exception(errorMsg)

    def module_error(self, error):
        errorMsg = f"Module \"{self.__id}\": {error}"
        Logger.log(LogMessageSeverityLevel.ERROR, errorMsg, LogMessageEntityType.MODULE, self.__id)

    def module_warning(self, warning):
        warningMsg = f"Module \"{self.__id}\": {warning}"
        Logger.log(LogMessageSeverityLevel.WARNING, warningMsg, LogMessageEntityType.MODULE, self.__id)

    def module_info(self, info):
        infoMsg = f"Module \"{self.__id}\": {info}"
        Logger.log(LogMessageSeverityLevel.INFO, infoMsg, LogMessageEntityType.MODULE, self.__id)

    def module_debug(self, dbg):
        dbgMessage = f"Module \"{self.__id}\": {dbg}"
        Logger.log(LogMessageSeverityLevel.DEBUG_VERBOSE, dbgMessage, LogMessageEntityType.MODULE, self.__id)

    def start(self, subscriber_publisher, properties, main_thread_runnables_queue):
        if self.__is_initialized:
            self.module_error("Initialize called twice!")
            return False

        Logger.log_info("Module start called")
        self.__subscriber_publisher = subscriber_publisher

        self.__runnable_dispatcher = RunnableDispatcher(main_thread_runnables_queue)

        if not self.__runnable_dispatcher.start():
            self.module_error("Failed to start RunnableDispatcher.")
            return False

        self.__is_initializing = True
        self.__is_initialized = False

        function_runnable = FunctionRunnableWithParams(self.__initialize_internal)
        function_runnable.set_params(properties)

        self.__runnable_dispatcher.add_runnable(
            ScheduledRunnable(
                function_runnable,
                ScheduleOnce(datetime.now())
            )
        )       

        while not self.__is_initialized:
            time.sleep(1)


        self.__is_initializing = False
        self.__subscriber_publisher = None
        return True

    def __initialize_internal(self, properties):
        Logger.log_info("Initialize internal called")
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

    def publish(self, channel_name, channel_type_example_instance):
        if not isinstance(channel_name, str):
            raise TypeError("Module.publish failed. Expected channel_name to be of type \"str\" (string) but got \"{}\"".format(type(channel_name)))

        if not self.__is_initializing:
            self.module_error(f'Cannot publish channel "{channel_name}". Publishing is only allowed during initialization (i.e., the first call of the initialize function).')
            return Channel.new_invalid_channel(channel_name)

        return self.__subscriber_publisher.publish(channel_type_example_instance, self, channel_name)

    def subscribe(self, channel_name, channel_type_example_instance, callback):
        if not isinstance(channel_name, str):
            raise TypeError("Module.subscribe failed. Expected channel_name to be of type \"str\" (string) but got \"{}\"".format(type(channel_name)))


        if not self.__is_initializing:
            self.module_error(f'Cannot subscribe channel "{channel_name}". Subscribing is only allowed during initialization (i.e., the first call of the initialize function).')
            return Channel.new_invalid_channel(channel_name)

        subscriber = Subscriber(channel_type_example_instance, callback, self.__runnable_dispatcher)
        return self.__subscriber_publisher.subscribe(channel_type_example_instance, self,  channel_name, subscriber)


    def register_periodic_function(self, name, callback, interval_timedelta):
        self.register_periodic_function_with_start_time(name, callback, interval_timedelta, datetime.now() + interval_timedelta)

    def register_periodic_function_with_start_time(self, name, callback, interval_timedelta, start_time):
        if interval_timedelta.total_seconds()*1000 == 0:
            self.module_error(f"Error in registerPeriodicFunction: Cannot register periodic function \"{name}\" with a period of 0 milliseconds.")

        if name in self.__timers:
            self.module_error(f"Tried to register function with name \"{name}\", but a periodic function with the same name was already registered before.")

        function_runnable = FunctionRunnable(callback)
        runnable = ScheduledRunnable(
            runnable=function_runnable,
            schedule=ScheduleRepeatedIntervall(start_time, interval_timedelta))

        self.__timers[name] = runnable
        self.__runnable_dispatcher.add_runnable(runnable)
        Logger.log_info(f"Registered periodic runnable {name}")

    def register_scheduled_function(self, name, function, start_time):
        if start_time < datetime.now():
            self.module_warning(f"Failed to schedule function \"{name}\" at time {start_time.strftime('%d.%m.%y - %H:%M:%S')}. "
                               f"The time is in the past. It is now: {datetime.now().strftime('%d.%m.%y - %H:%M:%S')}")

        function_runnable = FunctionRunnable(function)
        runnable = ScheduledRunnable(
            runnable=function_runnable,
            schedule=ScheduleOnce(start_time))

        if name in self.__timers:
            self.__timers[name].runnable.invalidate()

        self.__timers[name] = runnable
        self.__runnable_dispatcher.add_runnable(runnable)

    def unregister_periodic_function(self, name):
        if name not in self.__timers:
            self.module_error(f"Error, tried to unregister periodic function \"{name}\" but function was not found in the list of registered timers. "
                             f"Was a function with this name ever registered before?")
        self.__timers[name].runnable.invalidate()
        del self.__timers[name]

    def unregister_all_periodic_functions(self):
        for entry in self.__timers.values():
            entry.runnable.invalidate()
        self.__timers.clear()

    def shutdown(self):
        self.__is_terminating = True

        function_runnable = FunctionRunnable(self.terminate_internal)

        self.__runnable_dispatcher.add_runnable(
            ScheduledRunnable(
                function_runnable,
                ScheduleOnce(datetime.now())
            )
        )       
        while self.__is_terminating:
            time.sleep(1)


        Logger.log_info("Runnable dispatcher stop 1")
        self.__runnable_dispatcher.stop()
        Logger.log_info("Runnable dispatcher stop 2")
        self.__is_initialized = False
        Logger.log_info("Runnable dispatcher stop 3")

    def terminate_internal(self):
        Logger.log_info("Unregistering all periodic functions")
        self.unregister_all_periodic_functions()
        Logger.log_info("Calling terminate")
        self.terminate()
        Logger.log_info("Terminated")
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