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

from dispatch.proto.sensor_data_types_pb2 import AccelerationData
from dispatch.proto.sensor_data_types_pb2 import HeartRateData
from dispatch.proto.sensor_data_types_pb2 import BatteryData



def load_acceleration_data(path):
    data = AccelerationData()

    with open(path, mode="rb") as file:
        contents = file.read()

        if not data.ParseFromString(contents):
            return None
        return data
            


def load_concat_acceleration_data_from_multiple_files(path_list):

    concatenated_data = AccelerationData()

    for path in path_list:
        data = load_acceleration_data(path)

        if(data is None):
            return None

        concatenated_data.MergeFrom(data)

    return concatenated_data



def load_heartrate_data(path):
    data = HeartRateData()

    with open(path, mode="rb") as file:
        contents = file.read()

        if not data.ParseFromString(contents):
            return None
        return data
            


def load_concat_heartrate_data_from_multiple_files(path_list):

    concatenated_data = HeartRateData()

    for path in path_list:
        data = load_heartrate_data(path)

        if(data is None):
            return None

        concatenated_data.MergeFrom(data)

    return concatenated_data




def load_battery_data(path):
    data = BatteryData()

    with open(path, mode="rb") as file:
        contents = file.read()

        if not data.ParseFromString(contents):
            return None
        return data
            


def load_concat_battery_data_from_multiple_files(path_list):

    concatenated_data = BatteryData()

    for path in path_list:
        data = load_battery_data(path)

        if(data is None):
            return None

        concatenated_data.MergeFrom(data)

    return concatenated_data
