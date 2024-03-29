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
