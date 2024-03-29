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



import 'dart:async';

import 'package:claid/dispatcher.dart';
import 'package:claid/mocks.dart';
import 'package:claid/module.dart';

import 'package:test/test.dart';

// ModuleClassName, module ids and channel ids
//
// Schedule via sensor_value:
//     200
//     400
//     600
//     ...
//    2000  ==> 10 values
// Scheduled task (rescheduled once)
//
//     500
//    1000  ==> 2 value (List with 5 / 10 values)
//
// Triggered measurements
//
//      50
//     250
//     450  ==> 3 values
//
const moduleClassName = 'MyTestModuleOne';
const moduleInstanceId = 'MyTestModuleOne-instance';
const triggerReadChannel = 'trigger_read';
const sensorChannel = 'sensor_value';
const dailySensorChannel = 'daily_sensor_value';

// Set up a module environment.
final moduleEnv = MockModuleEnv(modules: <EnvModule>[
  MockModule(id: 'inputModule'),
  MockModule(id: 'outputModule'),
  UnderTest(
    id: moduleInstanceId,
    moduleClass: moduleClassName,
    properties: <String, String>{
      'interval': '200',
      'max_count': '10',
      'schedule_in': '500',
    },
  ),
], channels: <MockChannel>[
  MockChannel<bool>(
    id: triggerReadChannel,
    sourceModule: 'inputModule',
    targetModule: moduleInstanceId,
    generator: () => true,
    startMs: 50,
    periodMs: 200,
    count: 3,
  ),
  MockChannel(
    id: sensorChannel,
    sourceModule: moduleInstanceId,
    targetModule: 'outputModule',
  ),
  MockChannel(
    id: dailySensorChannel,
    sourceModule: moduleInstanceId,
    targetModule: 'outputModule',
  )
]);

void main() {
  test('test mock dispatcher', () async {
    final completer = Completer<void>();
    // Set up the module runtime.
    late MyTestModuleOne myMod;
    final factories = <String, FactoryFunc>{
      'MyTestModuleOne': () => myMod = MyTestModuleOne(completer),
    };

    // Initialize the module system.
    ModuleDispatcher? dispatcher;
    dispatcher =
        MockDispatcher(env: moduleEnv, onInit: () {}, onPacket: (pkt) {});
    await initModules(
        dispatcher: dispatcher,
        moduleFactories: moduleEnv.combinedFactories(factories));

    // Wait for a while for the events to trigger
    await completer.future;

    // Check if we have received the events in this order
    expect(myMod.allEvents.length, 13);
    expect(myMod.allEvents.map<double>((e) => e.value),
        [for (var i = 0; i < 13; i++) 42.0 + i]);
    expect(myMod.triggeredEvents.length, 3);
    expect(myMod.scheduledEvents.length, 2);

    // sensor events received by the output module
    expect(
        moduleEnv
            .getEventsForChannel(triggerReadChannel)
            ?.map<bool>((e) => e.boolVal)
            .toList(),
        <bool>[true, true, true]);

    // Compare if the results are correct.
  });
}

class TestEvent<T> {
  final T value;
  final DateTime timeStamp;
  const TestEvent(this.value, this.timeStamp);
}

// Implement a test module that
//
//     - reads a sensor periodically and publishes the data to a channel
//     - triggers a sensor read+pub based on channel input
//     - sends a summary of recorded sensor data once a day
//
class MyTestModuleOne extends Module {
  // Properties initialized during initialize(.)
  late int _intervalMs;
  late int _scheduleMs;
  late int _maxSensorCount;

  // Track the events for test verification.
  final periodicEvents = <TestEvent<double>>[];
  final scheduledEvents = <TestEvent<List<double>>>[];
  final triggeredEvents = <TestEvent<double>>[];
  final allEvents = <TestEvent>[];

  // keeps track of the number of measured sensor values.
  int _periodicCount = 0;

  // channels
  late SubscribeChannel<bool> _inputChannel;
  late PublishChannel<double> _outputChannel;
  late PublishChannel<List<double>> _dailyOutputChannel;

  // captures daily values for publication.
  final _dailyValues = <double>[];

  final Completer<void> _countReached;
  bool _rescheduled = false;

  // Names for setting up periodic and scheduled tasks.
  final periodicName = 'periodic_read';
  final scheduledName = 'scheduled_push';
  final unusedScheduleName = 'unused_schedule';

  MyTestModuleOne(this._countReached);

  @override
  void initialize(Map<String, String> props) {
    // Parse the properties
    _intervalMs = parseInt(props['interval']);
    _maxSensorCount = parseInt(props['max_count']);
    _scheduleMs = parseInt(props['schedule_in']);

    // register periodic functions
    registerPeriodicFunction(periodicName, Duration(milliseconds: _intervalMs),
        () async {
      periodicEvents.add(await _readSensor());
      _periodicCount++;
      if (_periodicCount >= _maxSensorCount) {
        unregisterPeriodicFunction(periodicName);
        _countReached.complete();
      }
    });

    // register a scheduled function
    final targetTime = DateTime.now().add(Duration(milliseconds: _scheduleMs));
    registerScheduledFunction(scheduledName, targetTime, _dailyPush);

    // Just to test whether the un-registration works.
    registerScheduledFunction(unusedScheduleName,
        DateTime.now().add(const Duration(seconds: 3600)), () {});
    unregisterScheduledFunction(unusedScheduleName);
    registerPeriodicFunction(
        unusedScheduleName, const Duration(milliseconds: 42), () {});
    unregisterPeriodicFunction(unusedScheduleName);

    // Set up a subscription.
    _inputChannel = subscribe<bool>(triggerReadChannel, true)
      ..onMessage(_triggerRead);

    // Set up a publish channel.
    _outputChannel = publish<double>(sensorChannel, 1.0);
    _dailyOutputChannel = publish<List<double>>(dailySensorChannel, <double>[]);
  }

  Future<TestEvent<double>> _readSensor() async {
    // simulate a sensor read

    final now = DateTime.now();
    double sensorVal = 42.0 + allEvents.length;
    _dailyValues.add(sensorVal);
    await Future.delayed(const Duration(milliseconds: 5));
    await _outputChannel.post(sensorVal);
    final ret = TestEvent(sensorVal, now);
    allEvents.add(ret);
    return ret;
  }

  Future<void> _dailyPush() async {
    final cpVals = List<double>.from(_dailyValues);
    _dailyValues.clear();
    await _dailyOutputChannel.post(cpVals);
    scheduledEvents.add(TestEvent(cpVals, DateTime.now()));

    // Re-register the function for the tomorrow.
    if (!_rescheduled) {
      _rescheduled = true;
      registerScheduledFunction(scheduledName,
          DateTime.now().add(const Duration(milliseconds: 1000)), _dailyPush);
    }
  }

  Future<void> _triggerRead(ChannelData<bool> payload) async {
    // fire and forget by ignoring the future.
    triggeredEvents.add(await _readSensor());
  }
}
