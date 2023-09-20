import 'package:claid_core/generated/claidservice.pb.dart';

import 'module.dart';
import 'dispatcher.dart';

// MockDispatcher allows to test higher level functions without
// instantiating the 'real' dispatcher, thus making tests hermetic.
class MockDispatcher implements ModuleDispatcher {
  @override
  Future<List<ModDescriptor>> getModuleList(List<String> moduleClasses) {
    // TODO: implement getModuleList
    throw UnimplementedError();
  }

  @override
  Future<void> initRuntime(Map<String, List<DataPackage>> modules) {
    // TODO: implement initRuntime
    throw UnimplementedError();
  }

  @override
  void shutdown() {
    // TODO: implement shutdown
  }

  @override
  // TODO: implement socketPath
  String get socketPath => throw UnimplementedError();

  @override
  bool start() {
    // TODO: implement start
    throw UnimplementedError();
  }
}

// Sensor
// Spirometer "Spirobank Smart™" (MIR company) will be used every day in the morning and in the evening to measure lung function of patients.
class MockSpirometer extends Module {
  @override
  void initialize(Map<String, String> properties) {
    // TODO: implement initialize
  }
}

// Sensor
// “Vivatmo me" (Bosch Healthcare Solutions GmbH) will be used once a week to measure FeNO.
class MockVivatmoMe extends Module {
  @override
  void initialize(Map<String, String> properties) {
    // TODO: implement initialize
  }
}

// Sensor
// Inhaler sensors or smart inhalers will to record the time and frequency of inhaler usage.
class MockInhalerSensor extends Module {
  @override
  void initialize(Map<String, String> properties) {
    // TODO: implement initialize
  }
}

// Sensor
// Smartphone sensor data will include a microphone (for cough detection), GPS, connection status, and battery status, which are all collected continuously.
class MockSmartphoneSensor extends Module {
  @override
  void initialize(Map<String, String> properties) {
    // TODO: implement initialize
  }
}

// Sensor
// Smartwatch sensor data will include heart rate, sleep duration, SpO2, breathing frequency, accelerometer, GPS, and battery status to collect data related to sleep quality.
class MockSmartwatchSensor extends Module {
  @override
  void initialize(Map<String, String> properties) {
    // TODO: implement initialize
  }
}
