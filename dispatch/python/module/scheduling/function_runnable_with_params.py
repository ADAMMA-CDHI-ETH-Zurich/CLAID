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

import types

from module.scheduling.runnable import Runnable

class FunctionRunnableWithParams(Runnable):
    def __init__(self, function=None):
        super().__init__()
        self.function = function
        self.stack = tuple()

    def run(self):

        self.function(*self.stack)

    def set_params(self, *params):
        self.stack = params

    def bind(self, func, object):
        if not callable(func):
            raise ValueError("The second argument must be a callable function/method.")
    
        if not hasattr(obj, func.__name__):
            raise AttributeError(f"The object does not have a method named {func.__name__}.")

        self.function = types.MethodType(func, object)

    def bind_with_params(self, func, obj, *args):
        self.bind(func, obj)
        self.set_params(*args)