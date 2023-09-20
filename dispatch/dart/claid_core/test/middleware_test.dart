import 'dart:async';
import 'dart:io';

import 'package:claid_core/dispatcher.dart';
import 'package:claid_core/generated/claidservice.pb.dart';
import 'package:claid_core/middleware.dart';
import 'package:flutter/widgets.dart';
import 'package:path/path.dart';

import 'package:test/test.dart';

// const String socketPath = '/tmp/testsock_dart.grpc';
const String configFile = './test/testdata/middleware_test_config.json';
const String hostId = 'alex_client';
const String userId = 'testUser';
const String deviceId = 'deviceId';

String getSocketPath(Directory tempDir) {
  return join(tempDir.path, 'claid-test.socket');
}

void main() {
  late Directory tempDir;

  setUp(() async {
    tempDir = await Directory.systemTemp.createTemp('dartclaid');
  });

  tearDown(() async {
    await tempDir.delete();
  });

  // test('test middleware bindings', () async {
  //   final socketPath = getSocketPath(tempDir);
  //   final middleWare =
  //       MiddleWareBindings(socketPath, configFile, userId, deviceId);

  //   expect(middleWare.ready, true);
  //   middleWare.shutdown();
  // });

  test('test client dispatcher', () async {
    final socketPath = getSocketPath(tempDir);
    final disp =
        ModuleDispatcher(socketPath, configFile, hostId, userId, deviceId);
    expect(disp.start(), true);

    final moduleClasses = <String>[
      'TestModuleOne',
      'TestModuleTwo',
    ];
    final modDesc = await disp.getModuleList(moduleClasses);
    print('\nMod Desc: $modDesc\n\n');

    // Set the channels
    final channels = <String, List<DataPackage>>{
      'test_mod_1': <DataPackage>[
        DataPackage(channel: 'RawNumber', sourceHostModule: 'test_mod_1'),
      ],
      'test_mod_2': <DataPackage>[
        DataPackage(channel: 'RawNumber', targetHostModule: 'test_mod_2'),
      ],
    };

    final outputController = StreamController<DataPackage>();
    final inputStream = await disp.initRuntime(channels, outputController);

    const maxPackets = 100;
    const outChannel = "mychannel";
    final outGoingPkts = <DataPackage>[];
    var counter = 0;

    final t = Timer.periodic(const Duration(milliseconds: 2000), (timer) {
      final pkt =
          DataPackage(channel: outChannel, numberVal: counter.toDouble());
      counter += 1;
      outGoingPkts.add(pkt);
      outputController.add(pkt);
      print('Package $counter sent.\n');
      if (outGoingPkts.length >= maxPackets) {
        timer.cancel();
      }
    });

    final completer = Completer<void>();
    final incoming = <DataPackage>[];
    late StreamSubscription<DataPackage> sub;
    sub = inputStream.listen((pkt) {
      print('Received package ${pkt.numberVal}');
      incoming.add(pkt);
      if (incoming.length == maxPackets) {
        sub.cancel();
        completer.complete();
      }
    });

    await completer.future;
    print('Done !');

    // Compare if the results are correct.

    disp.shutdown();
  });
}
