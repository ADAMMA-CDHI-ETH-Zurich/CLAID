import 'package:flutter_test/flutter_test.dart';

import 'package:claid_core/dispatcher.dart';

const String socketPath = '/tmp/testsock_dart.grpc';
const String configFile = './testdata/test.config';
const String userId = 'testUser';
const String deviceId = 'deviceId';

void main() {
  test('test client dispatcher', () async {
    final disp = DispatcherClient(socketPath, configFile, userId, deviceId);
    expect(disp.start(), true);

    // TODO: tests for these
    //    disp.getModuleList(moduleClasses);
    //    disp.initRuntime(module);

    disp.shutdown();
  });
}
