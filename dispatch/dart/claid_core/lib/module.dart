import 'dart:async';
import 'dart:io';

import './src/module_impl.dart' as impl;

typedef Callback = void Function();

typedef FactoryFunc = Module Function();

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
  static void registerModule(String moduleClassName, FactoryFunc factoryFn) {}

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
      String name, Duration period, Callback callback) {}
  void unregisterPeriodicFunction(String name) {}
  void registerScheduledFunction(
      String name, DateTime dateTime, Callback callback) {}
  void unregisterScheduledFunction(String name) {}

  SubscribeChannel<T> subscribe<T>(String channelId) {
    return impl.SubChannelImpl<T>();
  }

  PublishChannel<T> publish<T>(String channelId) {
    return impl.PubChannelImpl<T>();
  }
}

typedef ChannelCallback<T> = void Function(ChannelData<T> payload);

abstract class SubscribeChannel<T> {
  void onMessage(ChannelCallback<T> callback);
}

abstract class PublishChannel<T> {
  void post(T payload);
}

class ChannelData<T> {
  late T value;
  late DateTime timestamp;
}
