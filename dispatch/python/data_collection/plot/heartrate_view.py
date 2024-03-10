from claid.module import Module
from claid.dispatch.proto.sensor_data_types_pb2 import HeartRateData
from datetime import datetime

import matplotlib.pyplot as plt
import matplotlib
# matplotlib.use('QtAgg')
from matplotlib.animation import FuncAnimation
import numpy as np

import cv2
from logger.logger import Logger

class HeartRateView(Module):
    def __init__(self):
        super().__init__()

    @staticmethod
    def annotate_module(annotator):
        annotator.set_module_category("Visualization")
        annotator.set_module_description("A Module allowing to plot heart rate data and status over time..")
    
        annotator.describe_subscribe_channel("InputData", HeartRateData(), "Channel with incoming heart rate data.")


    def initialize(self, properties):
        self.max_points = 100
        self.hr = [0 for i in range(self.max_points)]
        self.times = [0 for i in range(self.max_points)]

        self.fig, self.ax = plt.subplots(1)
        
        self.window_name = "HeartRateView"

        self.ax.set_ylim(30, 160)
        self.line, = self.ax.plot(np.random.randn(100))
  
        self.fig.canvas.draw()

        self.input_channel = self.subscribe("InputData", HeartRateData(), self.onData)
        cv2.namedWindow(self.window_name, cv2.WINDOW_AUTOSIZE) 
        cv2.imshow(self.window_name, np.zeros((200,200,3)))
        cv2.waitKey(1)

        




    def onData(self, channel_data):
        data = channel_data.get_data()

        for sample in data.samples:
            self.hr.append(sample.hr)
            self.times.append(datetime.utcfromtimestamp(sample.unix_timestamp_in_ms / 1000))

        if len(self.hr) > self.max_points:
            self.hr = self.hr[-self.max_points:]

            self.times = self.times[-self.max_points:]


            self.line.set_ydata(self.hr)
            self.ax.draw_artist(self.ax.patch)
            self.ax.draw_artist(self.line)
     

            self.fig.canvas.draw()
            img_plot = np.array(self.fig.canvas.renderer.buffer_rgba())

            cv2.imshow(self.window_name, cv2.cvtColor(img_plot, cv2.COLOR_RGBA2BGR))

            cv2.waitKey(1)

    def terminate(self):
        Logger.log_info("HeartRateView is shutting down")
        cv2.destroyWindow(self.window_name)

        # if self.ctr == None:
        #     self.ctr = 0
        # self.ctr += 1
        # if self.ctr == 2:
        #     self.fig.canvas.draw()
        #     self.fig.canvas.flush_events()
        #     self.ctr = 0

