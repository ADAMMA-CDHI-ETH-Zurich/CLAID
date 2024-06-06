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
import 'package:claid/src/module_impl.dart' as impl;
import 'package:claid/dispatcher.dart';
import 'properties.dart';
import 'module_manager.dart';
import 'channel.dart';

// Helpers
// Duration parseDuration(String? durStr) {
//   return const Duration(microseconds: 5);
// }

int parseInt(String? intStr) {
  return int.parse(intStr!);
}

double parseDouble(String? floatStr) {
  return double.parse(floatStr!);
}

// DateTime dateTimeFromMidnight(Duration delta) {
//   // TODO: fix
//   return DateTime.now().add(delta);
// }

typedef FactoryFunc = Module Function();

Future<void> initModules(
    {required ModuleDispatcher dispatcher,
    required Map<String, FactoryFunc> moduleFactories}) async {
  final manager = ModuleManager(dispatcher, moduleFactories);
  await manager.start();
}

typedef RegisteredCallback = FutureOr<void> Function();

///  Lifecycle of modules
/// * Factory function is registered together with a module_class name
/// * System creates the module instance
/// * initialize is called which has to do the following:
///         - parse properties to extract command line params
///         - set up periodic functions
///         - Set up channels for subscription and publishing
/// * When the module is no longer needed terminate is called

// Modules is the base class of all modules.
abstract class Module {
  // initialize is called after the Module is created.
  // It must do the following:
  //         - parse properties to extract module params
  //         - set up periodic functions
  //         - Set up channels for subscription and publishing
  void initialize(Properties properties);

  // Initialize is called to wind down the module (like a destructor).
  void terminate() {}

  // Register a function that is called periodically.
  void registerPeriodicFunction(
          String name, Duration period, RegisteredCallback callback) =>
      _scheduler.registerPeriodicFunction(_modId, name, period, callback);

  void unregisterPeriodicFunction(String name) =>
      _scheduler.unregisterPeriodicFunction(_modId, name);

  void registerScheduledFunction(
          String name, DateTime dateTime, RegisteredCallback callback) =>
      _scheduler.registerScheduledFunction(_modId, name, dateTime, callback);

  void unregisterScheduledFunction(String name) =>
      _scheduler.unregisterScheduledFunction(_modId, name);

  Channel<T> subscribe<T>(String channelId, T instance, ChannelCallback<T> callback) 
  {
    _assertLifeCycle(impl.Lifecycle.initializing);
    return ModuleManager.instance
        .getSubscribeChannel<T>(_modId, channelId, instance, callback);
  }

  Channel<T> publish<T>(String channelId, T instance) 
  {
    _assertLifeCycle(impl.Lifecycle.initializing);
    return ModuleManager.instance
        .getPublishChannel<T>(_modId, channelId, instance);
  }

  set moduleId(String id) {
    _modId = id;
    _assertLifeCycle(impl.Lifecycle.created);
  }

  void _assertLifeCycle(impl.Lifecycle lc) {
    final actual = ModuleManager.instance.lifecycle(_modId);
    if (actual != lc) {
      throw AssertionError(
          'invalid lifecycle ${ModuleManager.instance.lifecycle(_modId)}');
    }
  }

  String _modId = "";
  final _scheduler = impl.Scheduler();
}


