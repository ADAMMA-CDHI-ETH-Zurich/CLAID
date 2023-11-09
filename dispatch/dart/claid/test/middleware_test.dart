/***************************************************************************
* Copyright (C) 2023 ETH Zurich
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
import 'dart:io';

import 'package:claid/dispatcher.dart';
import 'package:claid/generated/claidservice.pb.dart';
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
    await tempDir.delete(recursive: true);
  });

  test('test client dispatcher', () async {
    final socketPath = getSocketPath(tempDir);
    final disp =
        ModuleDispatcher(socketPath, configFile, hostId, userId, deviceId);
    expect(disp.start(), true);
    const channelID = 'RawNumber';

    final moduleClasses = <String>[
      'TestModuleOne',
      'TestModuleTwo',
    ];

    await disp.getModuleList(moduleClasses);

    // Set the channels
    const testModOne = 'test_mod_1';
    const testModTwo = 'test_mod_2';
    final channels = <String, List<DataPackage>>{
      testModOne: <DataPackage>[
        DataPackage(
            channel: channelID, sourceHostModule: testModOne, numberVal: 99),
      ],
      testModTwo: <DataPackage>[
        DataPackage(
            channel: channelID, targetHostModule: testModTwo, numberVal: 99),
      ],
    };

    final outputController = StreamController<DataPackage>();
    final inputStream = await disp.initRuntime(channels, outputController);

    const maxPackets = 5;

    final completer = Completer<void>();
    final incoming = <DataPackage>[];
    late StreamSubscription<DataPackage> sub;

    // TODO: enable shutdown once it works reliably.
    // Future<void> shutDown() async {
    //   await outputController.close();
    //   disp.closeRuntime();
    //   completer.complete();
    // }

    sub = inputStream.listen((pkt) async {
      incoming.add(pkt);
      if (incoming.length >= maxPackets) {
        // outputController.add(DataPackage(controlVal))
        await outputController.close();
        await Future.delayed(const Duration(milliseconds: 10));
        await sub.cancel();
        completer.complete();
      }
    }, onError: (err) {
      fail((err is StreamingError) ? (err.message) : err.toString());
    });

    final outGoingPkts = <DataPackage>[];
    var counter = 0;

    Timer.periodic(const Duration(milliseconds: 500), (timer) {
      final pkt = DataPackage(
        channel: channelID,
        sourceHostModule: testModOne,
        numberVal: counter.toDouble(),
      );
      outGoingPkts.add(pkt);
      outputController.add(pkt);
      counter += 1;
      if (outGoingPkts.length >= maxPackets) {
        timer.cancel();
      }
    });

    await completer.future;
    await disp.closeRuntime();

    // Compare if the results are correct.
    // TODO: shutdown currently hangs indefinitely. But that could be
    // a problem with gRPC or how it is used in the middleware.
    // disp.shutdown();
  });
}
