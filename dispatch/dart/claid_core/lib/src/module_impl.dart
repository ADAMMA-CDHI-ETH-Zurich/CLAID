import 'dart:async';
import 'dart:collection';

import 'package:claid_core/dispatcher.dart';
import 'package:claid_core/generated/claidservice.pb.dart';
import 'package:claid_core/mocks.dart';

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
  static ModuleManager? _manager;
  static ModuleManager get instance => _manager!;

  final ModuleDispatcher _dispatcher;
  final Map<String, FactoryFunc> _factories;
  final _moduleMap = <String, ModuleState>{};
  final _modChanMap =
      <String, List<DataPackage>>{}; // moduleId -> (channel, src, target, type)
  final _chanInstances = <String, dynamic>{}; // chanId => instance

  final _receiverMap = <String, ReceiverFunc>{}; // channelId -> function
  final _typeMapper = TypeMapping();
  StreamSubscription<DataPackage>? _inputSub;
  StreamController<DataPackage>? _outputCtrl;
  bool _outputPaused = true;
  final _pausedQueue = Queue<Completer<void>>();

  // factory ModuleManager(ModuleDispatcher disp, List<FactoryFunc> factories) {}
  factory ModuleManager(
      ModuleDispatcher dispatcher, Map<String, FactoryFunc> factories) {
    _manager = _manager ?? ModuleManager._internal(dispatcher, factories);
    return _manager!;
  }

  ModuleManager._internal(this._dispatcher, this._factories);

  Lifecycle lifecycle(String moduleId) =>
      _moduleMap[moduleId]?.lifecycle ?? Lifecycle.unknown;

  Future<void> start() async {
    if (!_dispatcher.start()) {
      throw AssertionError('ModuleDispatcher is not ready');
    }

    // Get the list of modules and instantiate + initialize them.
    final modDesc = await _dispatcher.getModuleList(_factories.keys.toList());
    _instantiateModules(modDesc);
    _initializeModules();
    _outputCtrl = StreamController<DataPackage>(
      onListen: _resumeOutput,
      onPause: _pauseOutput,
      onResume: _resumeOutput,
      onCancel: _pauseOutput,
    );

    final inStream = await _dispatcher.initRuntime(_modChanMap, _outputCtrl!);
    _inputSub = inStream.listen(_handleInputMessage,
        onDone: _cancelSubscription,
        onError: _handleSubscriptionErrors,
        cancelOnError: false);
  }

  SubscribeChannel<T> getSubscribeChannel<T>(
      String moduleId, String channelId, T inst) {
    final mutator = _setupChannel<T>(moduleId, channelId, inst, false);
    final subChan = SubChannelImpl<T>(mutator);
    final modChanId = _combineIds(moduleId, channelId);
    _receiverMap[modChanId] = subChan._receivedMsg;
    print('Sub: $moduleId : $channelId ==> $inst');
    return subChan;
  }

  PublishChannel<T> getPublishChannel<T>(
      String moduleId, String channelId, T inst) {
    final mutator = _setupChannel<T>(moduleId, channelId, inst, true);
    final pubChan = PubChannelImpl<T>(moduleId, channelId, mutator, this);
    _chanInstances[channelId] = inst;
    print('Pub: $moduleId : $channelId');
    return pubChan;
  }

  dynamic getInstanceForChannel(String channelId) {
    return _chanInstances[channelId]!;
  }

  void _instantiateModules(List<ModDescriptor> modDesc) {
    for (var mod in modDesc) {
      final fn = _factories[mod.moduleClass];
      if (fn == null) {
        throw UnsupportedError(
            "module factory ${mod.moduleClass} is not registered");
      }
      final instance = fn();
      _moduleMap[mod.moduleId] =
          ModuleState(instance, mod.properties, Lifecycle.created);
      instance.moduleId = mod.moduleId;
    }
  }

  void _initializeModules() {
    // If the dispatcher is mocked
    var moduleOrder = _dispatcher.getModuleOrder(_moduleMap.keys.toList());
    for (var key in moduleOrder) {
      final modState = _moduleMap[key]!;
      modState.lifecycle = Lifecycle.initializing;
      modState.instance.initialize(modState.props);
      modState.lifecycle = Lifecycle.initialized;
    }
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

  // Process incoming packages.
  void _handleInputMessage(DataPackage pkt) {
    final modChanId = _combineIds(pkt.targetHostModule, pkt.channel);
    final recv = _receiverMap[modChanId];
    if (recv != null) {
      recv(pkt);
    } else {
      print("Unrecognized channel ${pkt.channel}");
    }
  }

  void _cancelSubscription() async {
    _inputSub?.cancel();
  }

  void _handleSubscriptionErrors(Object error, StackTrace trace) {
    print('Trace: $trace');
    _inputSub?.cancel();
  }

  void _pauseOutput() {
    _outputPaused = true;
  }

  Future<void> _resumeOutput() async {
    // empty the queue
    while (_pausedQueue.isNotEmpty) {
      final comp = _pausedQueue.removeFirst();
      await Future.delayed(Duration.zero, () {
        comp.complete();
      });
    }
    _outputPaused = false;
  }

  Future<void> _waitForResume() {
    final comp = Completer();
    _pausedQueue.add(comp);
    return comp.future;
  }

  // Send outgoing packages.
  Future<void> _publish(DataPackage pkt) async {
    if (_outputPaused) {
      await _waitForResume();
    }
    _outputCtrl!.add(pkt);
  }
}

class SubChannelImpl<T> implements SubscribeChannel<T> {
  final Mutator<T> _mutator;
  final _callbacks = <ChannelCallback<T>>[];

  SubChannelImpl(this._mutator);

  void _receivedMsg(DataPackage pkt) async {
    for (var callback in _callbacks) {
      // TODO: See if we can get the timestamp from the package instead
      // of reading it out here.
      final cData = ChannelData<T>(
        _mutator.getter(pkt),
        DateTime.now(),
        pkt.sourceUserToken,
      );
      await callback(cData);
    }
  }

  @override
  void onMessage(ChannelCallback<T> callback) {
    _callbacks.add(callback);
  }
}

class PubChannelImpl<T> implements PublishChannel<T> {
  final String _srcModuleId;
  final String _channelId;
  final Mutator<T> _mutator;
  final ModuleManager _manager;
  PubChannelImpl(
      this._srcModuleId, this._channelId, this._mutator, this._manager);

  @override
  Future<void> post(T payload) async {
    final pkt =
        DataPackage(sourceHostModule: _srcModuleId, channel: _channelId);
    _mutator.setter(pkt, payload);
    await _manager._publish(pkt);
  }
}

class Scheduler {
  final _periodic = <String, Timer>{};
  final _scheduled = <String, Timer>{};

  void registerPeriodicFunction(String modId, String regName, Duration period,
      RegisteredCallback callback) {
    String regId = _combineIds(modId, regName);
    if (_periodic.containsKey(regId) || regName.isEmpty) {
      throw ArgumentError(
          'Name for periodic function "$regId" already in use or empty');
    }
    _periodic[regId] = Timer.periodic(period, (timer) => callback());
  }

  void unregisterPeriodicFunction(String modId, String regName) {
    String regId = _combineIds(modId, regName);
    if (!_periodic.containsKey(regId)) {
      throw ArgumentError('Periodic function "$regId" not registered');
    }
    _periodic.remove(regId)?.cancel();
  }

  void registerScheduledFunction(String modId, String regName,
      DateTime targetTime, RegisteredCallback callback) {
    String regId = _combineIds(modId, regName);

    // Make sure the function has not been scheduled before.
    if (_scheduled.containsKey(regId) || regName.isEmpty) {
      throw ArgumentError(
          'Name for scheduled function "$regId" already in use or empty');
    }

    final now = DateTime.now();
    if (!targetTime.isAfter(now)) {
      throw ArgumentError('Scheduled event "$regId" must be in the future');
    }

    _scheduled[regId] = Timer(targetTime.difference(now), () async {
      _scheduled.remove(regId);
      await callback();
    });
  }

  void unregisterScheduledFunction(String modId, String regName) {
    String regId = _combineIds(modId, regName);
    if (!_scheduled.containsKey(regId)) {
      throw ArgumentError('S function "$regId" not registered');
    }
    _scheduled.remove(regId)?.cancel();
  }
}

String _combineIds(String modId, String secondary) => '$modId:$secondary';