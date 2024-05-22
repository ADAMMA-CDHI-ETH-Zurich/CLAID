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
