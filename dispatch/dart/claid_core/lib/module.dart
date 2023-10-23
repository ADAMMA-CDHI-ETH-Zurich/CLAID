import 'dart:async';
import './src/module_impl.dart' as impl;
import './dispatcher.dart';

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
  final manager = impl.ModuleManager(dispatcher, moduleFactories);
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
  void initialize(Map<String, String> properties);

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

  SubscribeChannel<T> subscribe<T>(String channelId, T instance) {
    _assertLifeCycle(impl.Lifecycle.initializing);
    return impl.ModuleManager.instance
        .getSubscribeChannel<T>(_modId, channelId, instance);
  }

  PublishChannel<T> publish<T>(String channelId, T instance) {
    _assertLifeCycle(impl.Lifecycle.initializing);
    return impl.ModuleManager.instance
        .getPublishChannel<T>(_modId, channelId, instance);
  }

  set moduleId(String id) {
    _modId = id;
    _assertLifeCycle(impl.Lifecycle.created);
  }

  void _assertLifeCycle(impl.Lifecycle lc) {
    final actual = impl.ModuleManager.instance.lifecycle(_modId);
    if (actual != lc) {
      throw AssertionError(
          'invalid lifecycle ${impl.ModuleManager.instance.lifecycle(_modId)}');
    }
  }

  String _modId = "";
  final _scheduler = impl.Scheduler();
}

typedef ChannelCallback<T> = FutureOr<void> Function(ChannelData<T> payload);

abstract class SubscribeChannel<T> {
  void onMessage(ChannelCallback<T> callback);
}

abstract class PublishChannel<T> {
  Future<void> post(T payload);
}

class ChannelData<T> {
  final T value;
  final DateTime timestamp;
  final String userId;
  const ChannelData(this.value, this.timestamp, this.userId);
}