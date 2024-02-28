from claid.module import Module
from claid.dispatch.proto.sensor_data_types_pb2 import GyroscopeData
from datetime import datetime

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import numpy as np


class GyroscopeView(Module):
    def __init__(self):
        super().__init__()
        self.xs = []
        self.ys = []
        self.zs = []
        self.times = []

        self.fig, self.axs = plt.subplots(3)

        self.lines = list()
        for ax in self.axs:
            ax.set_ylim(-2,2)
            line, = ax.plot(np.random.randn(200))
            self.lines.append(line)

        plt.show(block=False)
        self.max_points = 200
    
    def frame_generator(self):
        frame_number = 0
        while True:
            yield frame_number
            frame_number += 1

    def initialize(self, properties):
        self.input_channel = self.subscribe("InputData", GyroscopeData(), self.onData)


    def onData(self, channel_data):
        data = channel_data.get_data()

        for sample in data.samples:
            self.xs.append(sample.gyroscope_x)
            self.ys.append(sample.gyroscope_y)
            self.zs.append(sample.gyroscope_z)
            self.times.append(datetime.utcfromtimestamp(sample.unix_timestamp_in_ms / 1000))

        if len(self.xs) > self.max_points:
            self.xs = self.xs[-self.max_points:]
            self.ys = self.ys[-self.max_points:]
            self.zs = self.zs[-self.max_points:]
            self.times = self.times[-self.max_points:]


            for ax, line, data in zip(self.axs, self.lines, [self.xs, self.ys, self.zs]):

                line.set_ydata(data)
                ax.draw_artist(ax.patch)
                ax.draw_artist(line)
            self.fig.canvas.update()
            self.fig.canvas.flush_events()

        # if self.ctr == None:
        #     self.ctr = 0
        # self.ctr += 1
        # if self.ctr == 2:
        #     self.fig.canvas.draw()
        #     self.fig.canvas.flush_events()
        #     self.ctr = 0

