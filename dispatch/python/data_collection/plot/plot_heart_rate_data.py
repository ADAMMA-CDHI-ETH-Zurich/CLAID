import os
import xmltodict
import matplotlib.pyplot as plt
from datetime import datetime
from time import time
import matplotlib.dates as mdates
import numpy as np
from tqdm import tqdm
from dispatch.proto.sensor_data_types_pb2 import HeartRateData
import matplotlib
# matplotlib.rcParams.update({'font.size': 42})



def plot_heartrate_data(data: HeartRateData):

    heart_rate_samples = list()
    times = list()

    last_heart_rate_sample = 60
    for sample in data.samples:
        if(sample.hr == 0):
            heart_rate_samples.append(last_heart_rate_sample)
        else:
            heart_rate_samples.append(sample.hr)
            last_heart_rate_sample = sample.hr
        times.append(datetime.utcfromtimestamp(sample.unix_timestamp_in_ms/1000))




    print("subsampling x")
    # convert timestamp to datetime
    
    print("plotting")
    ax = plt.subplot(111)
    ax.plot(times, heart_rate_samples)
    # set datetime formatter for x axis
    xfmt = mdates.DateFormatter('%H:%M')
    ax.xaxis.set_major_formatter(xfmt)
    # set ticks every 30 mins 
    ax.xaxis.set_major_locator(mdates.HourLocator(interval=1))
    # set fontsize of ticks
    ax.tick_params(axis='x', which='major')
    #ax.set_xlim(datetime(year=RECORD_YEAR, day=RECORD_DAY,month=RECORD_MONTH,hour=0), datetime(year=RECORD_YEAR, day=RECORD_DAY+1,month=RECORD_MONTH,hour=0))
    ax.set_ylabel("Heart rate data")
    # rotate ticks for x axis
    plt.xticks(rotation=90)
    plt.grid()

    plt.show()