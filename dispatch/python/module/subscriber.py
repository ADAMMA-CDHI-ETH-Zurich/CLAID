from module.abstract_subscriber import AbstractSubscriber
from module.scheduling.runnable_dispatcher import RunnableDispatcher

from module.scheduling.schedule_once import ScheduleOnce
from module.channel_data import ChannelData
from module.type_mapping.mutator import Mutator
from module.type_mapping.type_mapping import TypeMapping
from module.scheduling.function_runnable_with_params import FunctionRunnableWithParams
from module.scheduling.scheduled_runnable import ScheduledRunnable
from datetime import datetime
class Subscriber(AbstractSubscriber):
    def __init__(self, data_type_example_instance, callback, callback_dispatcher):
        self.callback = callback
        self.callback_dispatcher = callback_dispatcher
        self.mutator = TypeMapping.get_mutator(data_type_example_instance)

    def invoke_callback(self, data):
        function_runnable = FunctionRunnableWithParams(self.callback)
        function_runnable.set_params(data)

        
        self.callback_dispatcher.add_runnable(
            ScheduledRunnable(
                function_runnable,
                ScheduleOnce(datetime.now())
            )
        )

    def on_new_data(self, package):

        data = self.mutator.get_package_payload(package)
        channel_data = ChannelData( 
            data,
            datetime.fromtimestamp(package.unix_timestamp_ms / 1000.0),
            package.source_user_token
        )

        self.invoke_callback(channel_data)
