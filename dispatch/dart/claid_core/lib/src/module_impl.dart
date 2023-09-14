import 'dart:async';
import 'dart:io';

import "../module.dart";

class SubChannelImpl<T> extends SubscribeChannel<T> {
  @override
  void onMessage(ChannelCallback<T> callback) {}
}

class PubChannelImpl<T> extends PublishChannel<T> {
  void post(T payload) {}
}

class ModuleRegistry {
  final _factories = Map<String, FactoryFunc>();
}
