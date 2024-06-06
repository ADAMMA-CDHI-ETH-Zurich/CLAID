import 'module.dart';
import 'package:claid/src/module_impl.dart';
import 'package:claid/dispatcher.dart';
import 'package:claid/generated/claidservice.pb.dart';
import 'package:claid/generated/google/protobuf/struct.pb.dart';
import 'properties.dart';

import 'module.dart';
import 'type_mapping.dart';

import 'channel_access_rights.dart';
import 'channel.dart';

import 'dart:async';
import 'dart:collection';


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
    // if (!_dispatcher.start()) {
    //   throw AssertionError('ModuleDispatcher is not ready');
    // }

    // Get the list of modules and instantiate + initialize them.
    print("Dart ModuleManager getting module list ${_factories.keys.toList()}");
    final modDesc = await _dispatcher.getModuleList(_factories.keys.toList());
    print("Dart ModuleManager got module list ${modDesc}");

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



  Channel<T> getSubscribeChannel<T>(
      String moduleId, String channelId, T inst, ChannelCallback<T> callback) {
    final mutator = _setupChannel<T>(moduleId, channelId, inst, false);
    final subChan = Channel<T>(moduleId, channelId, mutator, this, ChannelAccessRights.READ, callback);
    final modChanId = combineIds(moduleId, channelId);
    _receiverMap[modChanId] = subChan.onMessageReceived;
    print('Sub: $moduleId : $channelId ==> $inst');
    return subChan;
  }

  Channel<T> getPublishChannel<T>(
      String moduleId, String channelId, T inst) {
    final mutator = _setupChannel<T>(moduleId, channelId, inst, true);
    final pubChan = Channel<T>(moduleId, channelId, mutator, this, ChannelAccessRights.WRITE, null);
    _chanInstances[channelId] = inst;
    print('Pub: $moduleId : $channelId');
    return pubChan;
  }

  dynamic getInstanceForChannel(String channelId) {
    return _chanInstances[channelId]!;
  }

  void _instantiateModules(List<ModDescriptor> modDesc) {
    for (var mod in modDesc) {
      print("Dart ModuleManager instantiating Module ${mod}");
      final fn = _factories[mod.moduleClass];
      if (fn == null) {
        
        throw UnsupportedError(
            "module factory ${mod.moduleClass} is not registered");
        continue;
      }
      final instance = fn();
      _moduleMap[mod.moduleId] =
          ModuleState(instance, mod.properties, Lifecycle.created);
      instance.moduleId = mod.moduleId;
      print("Dart ModuleManager instantiated Module ${mod}");

    }
  }

  void _initializeModules() {
    // If the dispatcher is mocked
      print("Dart ModuleManager initializing modules ");

    var moduleOrder = _dispatcher.getModuleOrder(_moduleMap.keys.toList());
    for (var key in moduleOrder) {
      final modState = _moduleMap[key]!;
      print("Dart ModuleManager initializing module ${modState} ");
      modState.lifecycle = Lifecycle.initializing;
      modState.instance.initialize(Properties(modState.props));
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
      chanPkt.sourceModule = moduleId;
    } else {
      chanPkt.targetModule = moduleId;
    }
    mutator.setter(chanPkt, inst);
    _modChanMap[moduleId] = chanList..add(chanPkt);
    return mutator;
  }

  // Process incoming packages.
  void _handleInputMessage(DataPackage pkt) {
    final modChanId = combineIds(pkt.targetModule, pkt.channel);

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
    print("Pause output");
    _outputPaused = true;
  }

  Future<void> _resumeOutput() async {
    // empty the queue
    print("Resume output");
    while (_pausedQueue.isNotEmpty) {
          print("Resume output 2");

      final comp = _pausedQueue.removeFirst();
          print("Resume output 3");

      await Future.delayed(Duration.zero, () {
        comp.complete();
      });
          print("Resume output 4");

    }
        print("Resume output 5");

    _outputPaused = false;
  }

  Future<void> _waitForResume() {
    final comp = Completer();
    _pausedQueue.add(comp);
    return comp.future;
  }

  // Send outgoing packages.
  Future<void> publish(DataPackage pkt) async {
            print("Publish 0 ${_outputPaused}");

    if (_outputPaused) {
      await _waitForResume();
    }
            print("Publish 1 ${pkt}");
    print("${_outputCtrl}");
    _outputCtrl!.add(pkt);
          print("Publish 2");

  }

  Module? getModule(final String moduleId)
  {
    if(!_moduleMap.containsKey(moduleId))
    {
      return null;
    }

    ModuleState? state = _moduleMap[moduleId];

    // If state is null, return null; otherwise, return the instance property of state.
    return state?.instance;
  }
}