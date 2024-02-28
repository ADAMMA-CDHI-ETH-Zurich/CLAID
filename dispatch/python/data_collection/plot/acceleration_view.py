from claid.module import Module
from claid.dispatch.proto.sensor_data_types_pb2 import AccelerationData
from datetime import datetime

import matplotlib.pyplot as plt
import matplotlib
matplotlib.use('QtAgg')
from matplotlib.animation import FuncAnimation
import numpy as np

import cv2

class AccelerationView(Module):
    def __init__(self):
        super().__init__()

    
    def frame_generator(self):
        frame_number = 0
        while True:
            yield frame_number
            frame_number += 1

    def initialize(self, properties):
        self.xs = []
        self.ys = []
        self.zs = []
        self.times = []

        self.fig, self.axs = plt.subplots(3)
        self.max_points = 100

        self.lines = list()
        for ax in self.axs:
            ax.set_ylim(-2,2)
            line, = ax.plot(np.random.randn(100))
            self.lines.append(line)

  
        self.fig.canvas.draw()

        self.input_channel = self.subscribe("InputData", AccelerationData(), self.onData)
        cv2.namedWindow("AccelerationView", cv2.WINDOW_AUTOSIZE) 
        cv2.imshow("AccelerationView", np.zeros((200,200,3)))
        cv2.waitKey(1)


    def get_acceleration_data(self):
        return self.xs, self.ys, self.zs

    def onData(self, channel_data):
        data = channel_data.get_data()

        for sample in data.samples:
            self.xs.append(sample.acceleration_x)
            self.ys.append(sample.acceleration_y)
            self.zs.append(sample.acceleration_z)
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
     

            self.fig.canvas.draw()
            img_plot = np.array(self.fig.canvas.renderer.buffer_rgba())

            cv2.imshow('AccelerationView', cv2.cvtColor(img_plot, cv2.COLOR_RGBA2BGR))

            cv2.waitKey(1)

        # if self.ctr == None:
        #     self.ctr = 0
        # self.ctr += 1
        # if self.ctr == 2:
        #     self.fig.canvas.draw()
        #     self.fig.canvas.flush_events()
        #     self.ctr = 0

