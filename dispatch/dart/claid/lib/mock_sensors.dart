/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/



import 'module.dart';
import 'package:claid/properties.dart';
// Sensor
// Spirometer "Spirobank Smart™" (MIR company) will be used every day in the morning and in the evening to measure lung function of patients.
class MockSpirometer extends Module {
  @override
  void initialize(Properties properties) {
    // TODO: implement initialize
  }
}

// Sensor
// “Vivatmo me" (Bosch Healthcare Solutions GmbH) will be used once a week to measure FeNO.
class MockVivatmoMe extends Module {
  @override
  void initialize(Properties properties) {
    // TODO: implement initialize
  }
}

// Sensor
// Inhaler sensors or smart inhalers will to record the time and frequency of inhaler usage.
class MockInhalerSensor extends Module {
  @override
  void initialize(Properties properties) {
    // TODO: implement initialize
  }
}

// Sensor
// Smartphone sensor data will include a microphone (for cough detection), GPS, connection status, and battery status, which are all collected continuously.
class MockSmartphoneSensor extends Module {
  @override
  void initialize(Properties properties) {
    // TODO: implement initialize
  }
}

// Sensor
// Smartwatch sensor data will include heart rate, sleep duration, SpO2, breathing frequency, accelerometer, GPS, and battery status to collect data related to sleep quality.
class MockSmartwatchSensor extends Module {
  @override
  void initialize(Properties properties) {
    // TODO: implement initialize
  }
}
