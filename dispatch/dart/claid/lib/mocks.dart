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

import 'package:claid/src/module_impl.dart';
import 'package:ffi/ffi.dart';
import 'package:fixnum/fixnum.dart';

import 'package:claid/generated/claidservice.pb.dart';
import 'package:claid/generated/google/protobuf/timestamp.pb.dart';
import 'module.dart';
import 'dispatcher.dart';

class MockModuleEnv {
  /* *** Data members *** */
  final List<EnvModule> modules;
  final List<MockChannel> channels;
  Map<String, FactoryFunc>? _allFactories;
  List<String>? _moduleOrder;
  final _channelEvents = <String, List<DataPackage>>{};

  /* *** Methods *** */
  MockModuleEnv({required this.modules, required this.channels}) {
    _moduleOrder = _validateModulesAndChannels();
  }

  Map<String, FactoryFunc> get allFactories => _allFactories!;

  // Augments the factories of the modules under test with the factories for
  // the generated mock modules.
  Map<String, FactoryFunc> combinedFactories(
      Map<String, FactoryFunc> underTest) {
    final ret = <String, FactoryFunc>{};
    for (var mod in modules) {
      if (mod is MockModule) {
        ret[mod._moduleClass] = mod.getMockFactory(this);
      } else if (mod is UnderTest) {
        if (!underTest.containsKey(mod.moduleClass)) {
          throw ArgumentError(
              'moduleClass "${mod.moduleClass}" not found in factories');
        }
        ret[mod.moduleClass!] = underTest[mod.moduleClass]!;
      }
    }
    _allFactories = ret;
    return ret;
  }

  List<ModDescriptor> get moduleDescriptors {
    return modules
        .map<ModDescriptor>((e) => (e as _BaseMockModule).descriptor)
        .toList();
  }

  List<DataPackage>? getEventsForChannel(String channelId) =>
      _channelEvents[channelId];

  void _addChannelEvent(String channelId, DataPackage pkt) {
    _channelEvents[channelId] = (_channelEvents[channelId] ?? <DataPackage>[])
      ..add(pkt);
  }

  List<MockChannel> _incomingChannelsFor(String moduleId) {
    return channels.where((ch) => ch.targetModule == moduleId).toList();
  }

  List<MockChannel> _outgoingChannelsFor(String moduleId) {
    return channels.where((ch) => ch.sourceModule == moduleId).toList();
  }

  List<String> _validateModulesAndChannels() {
    // Gather the modules
    final modulesUnderTest = modules.whereType<UnderTest>().toList();
    final mockModules = modules.whereType<MockModule>().toList();

    // ensure every module id is unique
    final mutSet = modulesUnderTest.map((m) => m.id).toSet();
    final mcmSet = mockModules.map((m) => m.id).toSet();
    if ((mutSet.length != modulesUnderTest.length) ||
        (mcmSet.length != mockModules.length) ||
        ((mcmSet.length + mutSet.length) != modules.length)) {
      throw AssertionError(
          'The ids of mock modules and modules under test have to be unique');
    }

    // partition into modules that are writing out channels and modules
    // that take in channel data. They matter in terms of order in which they
    // are initialized.
    final outputMods = <String>{};
    final inputMods = <String>{};

    // ensure the graph is connected,
    for (var chan in channels) {
      if (chan.sourceModule == chan.targetModule) {
        throw AssertionError('MockChannels cannot have the same module as'
            'source and target');
      }

      if (!mutSet.contains(chan.sourceModule) &&
          !mutSet.contains(chan.targetModule)) {
        throw AssertionError('MockChannels have to originate or terminate in a'
            'module unter test');
      }

      if (!mcmSet.contains(chan.sourceModule) &&
          !mcmSet.contains(chan.targetModule)) {
        throw AssertionError('MockChannels must originate or terminate in a '
            'mock module');
      }

      if (outputMods.contains(chan.targetModule) ||
          inputMods.contains(chan.sourceModule)) {
        throw AssertionError('MockModules can only have incoming or outgoing '
            'channels, but not both');
      }
      if (mcmSet.contains(chan.sourceModule)) {
        outputMods.add(chan.sourceModule);
      }

      if (mcmSet.contains(chan.targetModule)) {
        inputMods.add(chan.targetModule);
      }
    }

    return mutSet.toList()
      ..addAll(outputMods)
      ..addAll(inputMods);
  }

  List<String> _getModuleOrder(List<String> proposedOrder) {
    final modOrder = _moduleOrder!.toSet();
    final contained = proposedOrder.every((m) => modOrder.contains(m));
    if ((!contained) || (modOrder.length != proposedOrder.length)) {
      throw AssertionError(
          'Sets ${_moduleOrder!} and $proposedOrder do not match !');
    }
    return _moduleOrder!;
  }
}

// Tagging class to enforce which types can be pass to MockEnv.
abstract class EnvModule {}

abstract class _BaseMockModule {
  final String id;
  final String? moduleClass;
  final Map<String, String>? properties;

  _BaseMockModule({required this.id, this.moduleClass, this.properties});

  ModDescriptor get descriptor;
}

// Configuration of a mocked module in ModuleEnv.
class MockModule extends _BaseMockModule implements EnvModule {
  late String _moduleClass;
  MockModule({id}) : super(id: id) {
    _moduleClass = 'ModClass_for_$id';
  }

  @override
  ModDescriptor get descriptor =>
      ModDescriptor(id, _moduleClass, <String, String>{});

  FactoryFunc getMockFactory(MockModuleEnv env) =>
      () => _MockModuleImpl(this, env);
}

// Configuration of a module under test.
class UnderTest extends _BaseMockModule implements EnvModule {
  UnderTest({id, required moduleClass, required properties})
      : super(id: id, moduleClass: moduleClass, properties: properties);

  @override
  ModDescriptor get descriptor =>
      ModDescriptor(super.id, super.moduleClass!, super.properties!);
}

typedef MockGenerator<T> = T Function();

class MockChannel<T> {
  final String id;
  final String sourceModule;
  final String targetModule;
  MockGenerator<T>? generator;
  final int startMs;
  final int periodMs;
  final int count;

  MockChannel({
    required this.id,
    required this.sourceModule,
    required this.targetModule,
    this.generator,
    this.startMs = 0,
    this.periodMs = 1000,
    this.count = 10,
  }) {
    generator = generator ?? _lookupInstance;
  }

  T _lookupInstance() => ModuleManager.instance.getInstanceForChannel(id) as T;
}

// class MockGenerator<T> {
//   final T Function() genFunc;
//   static MockGenerator noop = MockGenerator<void>(() => {});
//   const MockGenerator(this.genFunc);
// }

// MockDispatcher allows to test higher level functions without
// instantiating the 'real' dispatcher, thus making tests hermetic.
class MockDispatcher implements ModuleDispatcher {
  final MockModuleEnv env;
  FutureOr<void> Function()? onInit;
  FutureOr<void> Function(DataPackage)? onPacket;
  final _routerChannelMap = <String, DataPackage>{};

  MockDispatcher({required this.env, this.onInit, this.onPacket});

  Timestamp timeStampFromDateTime(DateTime dt) {
    final millis = dt.millisecondsSinceEpoch;
    return Timestamp(
        seconds: Int64(millis ~/ 1000), nanos: (millis % 1000) * 1000000);
  }

  @override
  Future<List<ModDescriptor>> getModuleList(List<String> moduleClasses) {
    // Just a sanity check to make sure the logic is correct.
    for (var modClass in moduleClasses) {
      if (!env.allFactories.containsKey(modClass)) {
        throw AssertionError('Unable to find $modClass in module factories');
      }
    }

    final modDescriptors = env.moduleDescriptors;

    return Future<List<ModDescriptor>>.delayed(
        Duration.zero, () => modDescriptors);
  }

  @override
  Future<Stream<DataPackage>> initRuntime(
      Map<String, List<DataPackage>> modules,
      StreamController<DataPackage> outputController) {
    // Build the map to route channels correctly from the mock env.
    for (var ch in env.channels) {
      _routerChannelMap[ch.id] = DataPackage(
          channel: ch.id,
          sourceModule: ch.sourceModule,
          targetModule: ch.targetModule);
    }

    return Future<Stream<DataPackage>>.delayed(
        Duration.zero, () => _mockRouterInputStream(outputController.stream));
  }

  Stream<DataPackage> _mockRouterInputStream(
      Stream<DataPackage> outputStream) async* {
    await onInit?.call();
    await for (var pkt in outputStream) {
      // Simulate a transmission time !
      await Future<void>.delayed(const Duration(milliseconds: 10));

      // Make sure we have a minimum of package information and augment it
      // as the real middleware would.
      _emulateMiddleware(pkt);

      // notify that there is a package being sent.
      await onPacket?.call(pkt);
      yield pkt;
      env._addChannelEvent(pkt.channel, pkt);
    }
  }

  // These are all the actions that are taken by the middleware if we
  // were to send the message over the channel.
  void _emulateMiddleware(DataPackage pkt) {
    // Check that the incoming package has at least the minimum information
    // necessary.
    if ((pkt.sourceModule.isEmpty) ||
        (pkt.channel.isEmpty) ||
        (pkt.whichPayloadOneof() == DataPackage_PayloadOneof.notSet)) {
      throw ArgumentError('Data packet nees a channel, a source and payload');
    }

    // augment the fields like the router would in the middleware.
    final chanSrcTgt = _routerChannelMap[pkt.channel];

    if (chanSrcTgt == null) {
      throw ArgumentError('provided channel ${pkt.channel} unknown');
    }

    if (chanSrcTgt.sourceModule != pkt.sourceModule) {
      throw ArgumentError(
          'source modules "${chanSrcTgt.sourceModule}" and "${pkt.sourceModule}" do not match');
    }

    final now = DateTime.now();
    pkt.targetModule = chanSrcTgt.targetModule;
    pkt.tracePoints.add(TracePoint(timeStamp: timeStampFromDateTime(now)));
  }

  @override
  void shutdown() {}

  @override
  bool start() => true;

  @override
  Future<void> closeRuntime() async {
    // TODO: close runtime
  }

  @override
  List<String> getModuleOrder(List<String> proposedModuleOrder) =>
      env._getModuleOrder(proposedModuleOrder);
}

class _MockModuleImpl extends Module {
  final MockModule _info;
  final MockModuleEnv env;
  final _received = <dynamic>[];
  final _subs = <SubscribeChannel>[];

  _MockModuleImpl(MockModule mockModule, this.env) : _info = mockModule;

  @override
  void initialize(Map<String, String> properties) {
    // Register the subscription channels
    for (var chan in env._incomingChannelsFor(_info.id)) {
      _subs.add(subscribe(chan.id, chan.generator!())
        ..onMessage((payload) => _received.add(payload)));
    }

    final now = DateTime.now();
    for (var chan in env._outgoingChannelsFor(_info.id)) {
      final pub = publish(chan.id, chan.generator!());
      registerScheduledFunction(
          'outoing_sched', now.add(Duration(milliseconds: chan.startMs)),
          () async {
        // Post the first message and set up periodic messaing
        var count = 1;
        await pub.post(chan.generator!());

        // Post messages periodically.
        registerPeriodicFunction(
            'outgoing_periodic', Duration(milliseconds: chan.periodMs),
            () async {
          await pub.post(chan.generator!());
          count++;
          if (count >= chan.count) {
            unregisterPeriodicFunction('outgoing_periodic');
          }
        });
      });
    }
  }
}
