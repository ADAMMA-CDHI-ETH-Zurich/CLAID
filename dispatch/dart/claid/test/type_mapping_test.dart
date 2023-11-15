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

import 'package:claid/generated/claidservice.pb.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:claid/src/type_mapping.dart';
import './generated/testmessages.pb.dart';

void testMapping<T>(
    TypeMapping tm, T instance, T testVal, T Function(DataPackage) getDirect) {
  final mut = tm.getMutator<T>(instance);
  final pkt = DataPackage();
  mut.setter(pkt, testVal);
  expect(testVal, getDirect(pkt));
  expect(testVal, mut.getter(pkt));
}

void main() {
  test('test basic type to message mapping', () async {
    final tm = TypeMapping();

    // Test double, bool and String
    testMapping(tm, 0.1, 5.0, (pkt) => pkt.numberVal);
    testMapping(tm, false, true, (pkt) => pkt.boolVal);
    testMapping(tm, '', 'something', (pkt) => pkt.stringVal);

    // Test double array, String array, Map<String, double>, Map<String, String>
    testMapping(
        tm, <double>[], <double>[5, 3.5, 7.9], (pkt) => pkt.numberArrayVal.val);
    testMapping(tm, <String>[], <String>['hello', 'world'],
        (pkt) => pkt.stringArrayVal.val);
    testMapping(
        tm,
        <String, double>{},
        <String, double>{'hello': 5.3, 'world': 42},
        (pkt) => pkt.numberMap.val);
    testMapping(
        tm,
        <String, String>{},
        <String, String>{'hello': 'my', 'world': 'friend'},
        (pkt) => pkt.stringMap.val);

    expect(() => testMapping<int>(tm, 0, 0, (pkt) => pkt.numberVal.toInt()),
        throwsArgumentError);

    // Test protobuf payload
    final expPayload = ExamplePayload(name: "john", value: 42);
    final mutProto = tm.getMutator<ExamplePayload>(ExamplePayload());
    final pkt = DataPackage();
    mutProto.setter(pkt, expPayload);
    expect(pkt.blobVal.codec, Codec.CODEC_PROTO);
    expect(pkt.blobVal.messageType, 'examplemsg.ExamplePayload');

    final actualPayload = mutProto.getter(pkt);
    expect(actualPayload, expPayload);
    expect(actualPayload.name, 'john');
    expect(actualPayload.value, 42);
  });
}
