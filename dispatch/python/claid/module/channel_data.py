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

class ChannelData:

    def __init__(self, data = None, timestamp=None, user_id = None):
        self.data = data
        self.timestamp = timestamp
        self.user_id = user_id

        self.valid = False

        if self.data is not None and self.timestamp is not None and self.user_id is not None:
            self.valid = True

    def get_data(self):
        return self.data

    def get_timestamp(self):
        return self.timestamp

    def get_user_id(self):
        return self.user_id

    def is_valid(self):
        return self.valid