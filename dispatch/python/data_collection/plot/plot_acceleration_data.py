import os
import matplotlib.pyplot as plt
from datetime import datetime
from time import time
import matplotlib.dates as mdates
import numpy as np
from tqdm import tqdm
import matplotlib


from dispatch.proto.sensor_data_types_pb2 import AccelerationData

def update_acceleration_data(data: AccelerationData, fig, axs):

    xs = list()
    ys = list()
    zs = list()
    times = list()

    for sample in data.samples:
        xs.append(sample.acceleration_x)
        ys.append(sample.acceleration_y)
        zs.append(sample.acceleration_z)
        times.append(datetime.utcfromtimestamp(sample.unix_timestamp_in_ms/1000))

    update_acceleration_data_raw(xs,ys,zs,times,fig,axs)


def update_acceleration_data_raw(xs,ys,zs,times,fig,axs):

    data = [xs,ys,zs]

    labels = ["X-axis", "Y-axis", "Z-axis"]
    cmap = plt.get_cmap("tab10")
    ctr = 0
    for ax, values, label in tqdm(zip(axs, data, labels)):
        line, = ax
        line.set_data(values)
        #ax.plot(times, values, label = label, color=cmap(ctr))

        # set datetime formatter for x axis
        xfmt = mdates.DateFormatter('%H:%M')
        ax.xaxis.set_major_formatter(xfmt)
            # set ticks every 30 mins 
        ax.xaxis.set_major_locator(mdates.HourLocator(interval=1))
            # set fontsize of ticks
        ax.tick_params(axis='x', which='major')
        #ax.set_xlim(datetime(year=RECORD_YEAR, day=RECORD_DAY,month=RECORD_MONTH,hour=0), datetime(year=RECORD_YEAR, day=RECORD_DAY+1,month=RECORD_MONTH,hour=0))
        ax.set_ylim(-2, 2)
        ax.grid()
        ax.set_ylabel(label)
        if(ctr != 2):
            plt.setp(axs[ctr].get_xticklabels(), visible = False)

        ctr+=1

        # ax.set_ylabel("Polar accelerometer")
    # rotate ticks for x axis
    plt.xticks(rotation=90)


    # fig.set_size_inches(20,10, forward=True)
    # fig.set_dpi(200)
    plt.pause(0.00000001)


def plot_acceleration_data(data: AccelerationData, blocking = False):
    fig, axs = plt.subplots(3)

    update_acceleration_data(data, fig=fig, axs=axs)

    plt.show(block=blocking)
    return fig, axs

