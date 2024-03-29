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

class ScheduleRepeatedIntervall(ScheduleDescription):
    def __init__(self, start_time, interval_timedelta):
        self.execution_time = start_time
        self.interval_timedelta = interval_timedelta

    def does_runnable_have_to_be_repeated(self):
        return True

    def update_execution_time(self):
        self.execution_time += self.interval_timedelta
        # Uncomment the next line if you want to print the next schedule time
        # print(self.execution_time.strftime("Next schedule %H:%M:%S"))

    def get_execution_time(self):
        return self.execution_time
