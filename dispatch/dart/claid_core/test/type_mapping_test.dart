import 'package:claid_core/generated/claidservice.pb.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:claid_core/src/type_mapping.dart';
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
