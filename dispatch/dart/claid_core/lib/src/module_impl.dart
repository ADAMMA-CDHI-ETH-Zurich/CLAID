import 'dart:async';

import 'package:claid_core/dispatcher.dart';
import 'package:claid_core/generated/claidservice.pb.dart';
import 'package:flutter/widgets.dart';
import 'package:grpc/grpc.dart';

import '../module.dart';
import 'type_mapping.dart';

enum Lifecycle {
  unknown,
  created,
  initializing,
  initialized,
  running,
  terminated
}

class ModuleState {
  final Module instance;
  final Map<String, String> props;
  Lifecycle lifecycle;
  ModuleState(this.instance, this.props, this.lifecycle);
}

typedef ReceiverFunc = void Function(DataPackage);

class ModuleManager {
  static late ModuleManager _manager;
  static get instance => _manager;

  final ModuleDispatcher _dispatcher;
  final Map<String, FactoryFunc> _factories;
  final _moduleMap = <String, ModuleState>{};
  final _modChanMap =
      <String, List<DataPackage>>{}; // channel -> (src,target, type)
  final _receiverMap = <String, ReceiverFunc>{};
  final _typeMapper = TypeMapping();

  // factory ModuleManager(ModuleDispatcher disp, List<FactoryFunc> factories) {}
  ModuleManager(this._dispatcher, this._factories) {}

  Lifecycle lifecycle(String moduleId) =>
      _moduleMap[moduleId]?.lifecycle ?? Lifecycle.unknown;

  Future<void> start() async {
    if (!_dispatcher.start()) {
      throw AssertionError('ModuleDispatcher is not ready');
    }

    // Get the list of modules and instantiate + initialize them.
    final modDesc = await _dispatcher.getModuleList(_factories.keys.toList());
    instantiateModules(modDesc);
    final chanList = initializeModules();
    await _dispatcher.initRuntime(_modChanMap);
  }

  void instantiateModules(List<ModDescriptor> modDesc) {
    for (var mod in modDesc) {
      final fn = _factories[mod.moduleClass];
      if (fn == null) {
        throw UnsupportedError(
            "module factory ${mod.moduleClass} is not registered");
      }
      final instance = fn();
      instance.moduleId = mod.moduleId;
      _moduleMap[mod.moduleId] =
          ModuleState(instance, mod.properties, Lifecycle.created);
    }
  }

  void initializeModules() {
    for (var m in _moduleMap.entries) {
      final s = _moduleMap[m];
      if (s != null) {
        s.lifecycle = Lifecycle.initializing;
        s.instance.initialize(s.props);
        s.lifecycle = Lifecycle.initialized;
      }
    }
  }

  SubscribeChannel<T> getSubscribeChannel<T>(
      String moduleId, String channelId, T inst) {
    final mutator = _setupChannel<T>(moduleId, channelId, inst, false);
    final subChan = SubChannelImpl<T>(mutator);
    _receiverMap[channelId] = subChan._receivedMsg;
    return subChan;
  }

  PublishChannel<T> getPublishChannel<T>(
      String moduleId, String channelId, T inst) {
    final mutator = _setupChannel<T>(moduleId, channelId, inst, true);
    final pubChan = PubChannelImpl<T>(channelId, mutator, this);
    return pubChan;
  }

  Mutator<T> _setupChannel<T>(
      String moduleId, String channelId, T inst, bool isSource) {
    final chanList = _modChanMap[moduleId] ?? <DataPackage>[];
    if (!chanList.every((element) => element.channel != channelId)) {
      throw AssertionError(
          'Channel "$channelId" already registered for module "$moduleId"');
    }

    final mutator = _typeMapper.getMutator(inst);
    final chanPkt = DataPackage(channel: channelId);
    if (isSource) {
      chanPkt.sourceHostModule = moduleId;
    } else {
      chanPkt.targetHostModule = moduleId;
    }
    mutator.setter(chanPkt, inst);
    _modChanMap[moduleId] = chanList..add(chanPkt);
    return mutator;
  }

  Future<void> publish(DataPackage pkt) async {
    // TODO: implement
  }
}

class SubChannelImpl<T> implements SubscribeChannel<T> {
  final Mutator<T> _mutator;
  final _callbacks = <ChannelCallback<T>>[];

  SubChannelImpl(this._mutator);

  void _receivedMsg(DataPackage pkt) {
    for (var callback in _callbacks) {
      final cData = ChannelData<T>(
          _mutator.getter(pkt), pkt.tracePoints.first.timeStamp.toDateTime());
      callback(cData);
    }
  }

  @override
  void onMessage(ChannelCallback<T> callback) {
    _callbacks.add(callback);
  }
}

class PubChannelImpl<T> implements PublishChannel<T> {
  final String _channelId;
  final Mutator<T> _mutator;
  final ModuleManager _manager;
  PubChannelImpl(this._channelId, this._mutator, this._manager);

  @override
  Future<void> post(T payload) async {
    final pkt = DataPackage(channel: _channelId);
    _mutator.setter(pkt, payload);
    await _manager.publish(pkt);
  }
}
