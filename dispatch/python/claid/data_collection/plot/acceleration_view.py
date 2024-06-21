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

from claid.module import Module
from claid.dispatch.proto.sensor_data_types_pb2 import AccelerationData
from datetime import datetime

import matplotlib.pyplot as plt
import matplotlib
# matplotlib.use('QtAgg')
from matplotlib.animation import FuncAnimation
import numpy as np

import cv2
from logger.logger import Logger

class AccelerationView(Module):
    def __init__(self):
        super().__init__()

    @staticmethod
    def annotate_module(annotator):
        annotator.set_module_category("Visualization")
        annotator.set_module_description("A Module allowing to plot 3 dimensional acceleration data (X, Y, Z axis) in realtime using matplotlib.")
    
        annotator.describe_subscribe_channel("InputData", AccelerationData(), "Channel with incoming acceleration data.")

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
        
        self.window_name = "AccelerationView"
        self.window_closed = False
        self.lines = list()
        for ax in self.axs:
            ax.set_ylim(-2*9.80655,2*9.80655)
            line, = ax.plot(np.random.randn(100))
            self.lines.append(line)

  
        self.fig.canvas.draw()

        self.input_channel = self.subscribe("InputData", AccelerationData(), self.onData)
        cv2.namedWindow(self.window_name, cv2.WINDOW_AUTOSIZE) 
        cv2.imshow(self.window_name, np.zeros((200,200,3)))

        


    def get_acceleration_data(self):
        return self.xs, self.ys, self.zs

    def onData(self, channel_data):

        if self.window_closed:
            return
        
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

            cv2.imshow(self.window_name, cv2.cvtColor(img_plot, cv2.COLOR_RGBA2BGR))

        if cv2.waitKey(1) == 27:
            self.window_closed = True
            cv2.destroyWindow(self.window_name)

 
    def terminate(self):
        Logger.log_info("AccelerationView is shutting down")

        if not self.window_closed:
            cv2.destroyWindow(self.window_name)

        # if self.ctr == None:
        #     self.ctr = 0
        # self.ctr += 1
        # if self.ctr == 2:
        #     self.fig.canvas.draw()
        #     self.fig.canvas.flush_events()
        #     self.ctr = 0

