from claid.module import Module
from claid.dispatch.proto.sensor_data_types_pb2 import GyroscopeData
from datetime import datetime

import matplotlib.pyplot as plt
import matplotlib
# matplotlib.use('QtAgg')
from matplotlib.animation import FuncAnimation
import numpy as np

import cv2
from logger.logger import Logger

class GyroscopeView(Module):
    def __init__(self):
        super().__init__()

    @staticmethod
    def annotate_module(annotator):
        annotator.set_module_category("Visualization")
        annotator.set_module_description("A Module allowing to plot 3 dimensional gyroscope data (X, Y, Z axis) in realtime using matplotlib.")
    
        annotator.describe_subscribe_channel("InputData", GyroscopeData(), "Channel with incoming acceleration data.")
    
    def frame_generator(self):
        frame_number = 0
        while True:
            yield frame_number
            frame_number += 1

    def initialize(self, properties):
        self.max_points = 100
        self.xs = [0 for i in range(self.max_points)]
        self.ys = [0 for i in range(self.max_points)]
        self.zs = [0 for i in range(self.max_points)]
        self.times = [0 for i in range(self.max_points)]

        self.fig, self.axs = plt.subplots(3)

        self.window_name = "GyroscopeView"


        self.lines = list()
        for ax in self.axs:
            ax.set_ylim(-2*9.80655,2*9.80655)
            line, = ax.plot(np.random.randn(100))
            self.lines.append(line)

  
        self.fig.canvas.draw()

        self.input_channel = self.subscribe("InputData", GyroscopeData(), self.onData)

        cv2.namedWindow(self.window_name, cv2.WINDOW_AUTOSIZE) 
        cv2.imshow(self.window_name, np.zeros((200,200,3)))
        cv2.waitKey(1)


    def get_acceleration_data(self):
        return self.xs, self.ys, self.zs

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
     

            self.fig.canvas.draw()
            img_plot = np.array(self.fig.canvas.renderer.buffer_rgba())

            cv2.imshow(self.window_name, cv2.cvtColor(img_plot, cv2.COLOR_RGBA2BGR))

            cv2.waitKey(1)

    def terminate(self):
        Logger.log_info("GyroscopeView is shutting down")
        cv2.destroyWindow(self.window_name)
        # if self.ctr == None:
        #     self.ctr = 0
        # self.ctr += 1
        # if self.ctr == 2:
        #     self.fig.canvas.draw()
        #     self.fig.canvas.flush_events()
        #     self.ctr = 0

