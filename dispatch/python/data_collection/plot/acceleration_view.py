from claid.module import Module
from claid.dispatch.proto.sensor_data_types_pb2 import AccelerationData
from datetime import datetime

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import numpy as np


class AccelerationView(Module):
    def __init__(self):
        super().__init__()
        self.xs = []
        self.ys = []
        self.zs = []
        self.times = []

        self.fig, (ax_x, ax_y, ax_z) = plt.subplots(3, 1, sharex=True)
        self.fig.suptitle('Acceleration Data')

        ax_x.set_ylim(-10, 10)
        ax_y.set_ylim(-10, 10)
        ax_z.set_ylim(-10, 10)    

        self.max_points = 100

        ax_x.set_xlim(0,self.max_points)
        ax_y.set_xlim(0,self.max_points)
        ax_z.set_xlim(0,self.max_points)

        self.line_x, = ax_x.plot(np.zeros(self.max_points), np.zeros(self.max_points), label='X-axis')
        self.line_y, = ax_y.plot(np.zeros(self.max_points), np.zeros(self.max_points), label='Y-axis')
        self.line_z, = ax_z.plot(np.zeros(self.max_points), np.zeros(self.max_points), label='Z-axis')

        ax_x.legend()
        ax_y.legend()
        ax_z.legend()
        self.ctr = 0
    
    def frame_generator(self):
        frame_number = 0
        while True:
            yield frame_number
            frame_number += 1

    def initialize(self, properties):
        self.input_channel = self.subscribe("InputData", AccelerationData(), self.onData)

    def update(self, frame):
        print("animation called")
        x_acceleration, y_acceleration, z_acceleration = self.get_acceleration_data()

        # Plotting
        self.line_x.set_data(range(len(x_acceleration)), x_acceleration)
        self.line_y.set_data(range(len(y_acceleration)), y_acceleration)
        self.line_z.set_data(range(len(z_acceleration)), z_acceleration)


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

        if self.ctr == None:
            self.ctr = 0
        self.ctr += 1
        if self.ctr == 2:
            self.fig.canvas.draw()
            self.fig.canvas.flush_events()
            self.ctr = 0

