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

import 'package:flutter_test/flutter_test.dart';
import 'package:claid/CLAID.dart';
import 'package:claid/module.dart';
import 'package:claid/module_factory.dart';

import 'dart:async';
import 'dart:io';
import 'package:path/path.dart';
const String configFile = './test/testdata/middleware_test_config.json';
const String hostId = 'alex_client';
const String userId = 'testUser';
const String deviceId = 'deviceId';

String getSocketPath(Directory tempDir) {
  return join(tempDir.path, 'claid-test.socket');
}


class MyClass extends Module
{
    @override
    void initialize(Map<String, String> props) 
    {

    }
}

void main() 
{
  test('Test starting the CLAID middleware via the CLAID class', () async {
    
    late Directory tempDir;

    setUp(() async {
        tempDir = await Directory.systemTemp.createTemp('dartclaid');
    });

    tearDown(() async {
        await tempDir.delete(recursive: true);
    });

    ModuleFactory moduleFactory = ModuleFactory();
    moduleFactory.registerClass("MyClass", () => MyClass());


    CLAID.start(getSocketPath(tempDir), configFile, hostId, userId, deviceId, moduleFactory);

    
  });
}
