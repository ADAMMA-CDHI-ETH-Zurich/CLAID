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

import 'package:flutter_test/flutter_test.dart';
import 'package:claid/CLAID.dart';
import 'package:claid/module/module.dart';
import 'package:claid/module/module_factory.dart';
import 'package:claid/module/module_manager.dart';

import 'dart:async';
import 'dart:io';
import 'package:path/path.dart';
import 'package:claid/module/properties.dart';

const String configFile = './test/testdata/middleware_test_config.json';
const String hostId = 'alex_client';
const String userId = 'testUser';
const String deviceId = 'deviceId';

String getSocketPath(Directory tempDir) {
  return join(tempDir.path, 'claid-test.socket');
}

class MyClass extends Module {
  @override
  void initialize(Properties props) {}
}

void main() {
  late Directory tempDir;

  setUp(() async {
    tempDir = await Directory.systemTemp.createTemp('dartclaid');
  });

  tearDown(() async {
    await tempDir.delete(recursive: true);
  });

  test('Test starting the CLAID middleware via the CLAID class', () async {
    // TODO: Fix this test !

    // ModuleFactory moduleFactory = ModuleFactory();
    // moduleFactory.registerClass("MyClass", () => MyClass());

    // CLAID.start(getSocketPath(tempDir), configFile, hostId, userId, deviceId,
    //     moduleFactory);
  });
}
