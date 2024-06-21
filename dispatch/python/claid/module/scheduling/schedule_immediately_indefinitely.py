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


from module.scheduling.schedule_description import ScheduleDescription
from datetime import datetime

class ScheduleImmediatelyIndefinitely(ScheduleDescription):
    def __init__(self, execution_time: datetime):
        self.execution_time = execution_time

    @classmethod
    def startNow(cls):
        return cls(datetime.now())

    def does_runnable_have_to_be_repeated(self):
        return True

    def update_execution_time(self):
        # Does not exist for ScheduleOnce.
        # A ScheduledRunnable with ScheduleDescription
        # of type ScheduleOnce is not supposed to be repeated.
        self.execution_time = datetime.now()

    def get_execution_time(self):
        return self.execution_time
