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
from claid.dispatch.proto.sensor_data_types_pb2 import HeartRateData, HeartRateStatus
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

            text = "Hr status: " + str(HeartRateStatus.Name(data.samples[-1].status))
            font = cv2.FONT_HERSHEY_SIMPLEX
            font_scale = 1
            font_thickness = 2

            # Get the size of the text
            text_size = cv2.getTextSize(text, font, font_scale, font_thickness)[0]

            # Calculate the position to place the text in the top middle
            text_x = (img_plot.shape[1] - text_size[0]) // 2
            text_y = text_size[1] + 10  # Add a margin from the top
            print("Text ", text)
            # Put the text on the image
            img_plot = cv2.putText(img_plot, text, (text_x, text_y), font, font_scale, (0, 0, 0), font_thickness)

            # Display the

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

