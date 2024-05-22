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

from dispatch.python.data_collection.load.load_sensor_data import * 
from data_collection.plot.plot_acceleration_data import * 
from data_collection.plot.plot_heart_rate_data import * 
from data_collection.plot.plot_battery_data import * 

paths = list()
for day in range(18, 21):
    for hour in range(0, 24):
        paths.append("/Users/planger/Documents/CLAID/CLAIDV2DataTest/{:02d}.01.24/acceleration_data_{:02d}.01.24_{:02d}.binary".format(day, day, hour))
        
data = load_concat_acceleration_data_from_multiple_files(paths)
plot_acceleration_data(data)
# paths = list()
# for day in range(18, 21):
#     for hour in range(0, 24):
#         paths.append("/Users/planger/Documents/CLAID/CLAIDV2DataTest/{:02d}.01.24/heart_rate_data_{:02d}.01.24_{:02d}.binary".format(day, day, hour))
        
# data = load_concat_heartrate_data_from_multiple_files(paths)
# plot_heartrate_data(data)

# paths = list()
# for day in range(18, 21):
#     for hour in range(0, 24):
#         paths.append("/Users/planger/Documents/CLAID/CLAIDV2DataTest/{:02d}.01.24/battery_data_{:02d}.01.24_{:02d}.binary".format(day, day, hour))
        
# data = load_concat_battery_data_from_multiple_files(paths)
# plot_battery_data(data)

exit(0)
