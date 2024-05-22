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

import os
import xmltodict
import matplotlib.pyplot as plt
from datetime import datetime
from time import time
import matplotlib.dates as mdates
import numpy as np
from tqdm import tqdm
from dispatch.proto.sensor_data_types_pb2 import BatteryData
import matplotlib
# matplotlib.rcParams.update({'font.size': 42})



def plot_battery_data(data: BatteryData):

    battery_samples = list()
    times = list()

    for sample in data.samples:
       
        battery_samples.append(sample.level)
        times.append(datetime.utcfromtimestamp(sample.unix_timestamp_in_ms/1000))




    print("subsampling x")
    # convert timestamp to datetime
    
    print("plotting")
    ax = plt.subplot(111)
    ax.plot(times, battery_samples)
    # set datetime formatter for x axis
    xfmt = mdates.DateFormatter('%H:%M')
    ax.xaxis.set_major_formatter(xfmt)
    # set ticks every 30 mins 
    ax.xaxis.set_major_locator(mdates.HourLocator(interval=1))
    # set fontsize of ticks
    ax.tick_params(axis='x', which='major')
    #ax.set_xlim(datetime(year=RECORD_YEAR, day=RECORD_DAY,month=RECORD_MONTH,hour=0), datetime(year=RECORD_YEAR, day=RECORD_DAY+1,month=RECORD_MONTH,hour=0))
    ax.set_ylabel("Battery data")
    # rotate ticks for x axis
    plt.xticks(rotation=90)
    plt.grid()

    plt.show()