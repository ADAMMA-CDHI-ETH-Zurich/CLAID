import 'package:claid_core/generated/claidservice.pb.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:claid_core/src/type_mapping.dart';
import './generated/testmessages.pb.dart';

void main() {
  test('test basic type to message mapping', () async {
    final tm = TypeMapping();
    var pkt = DataPackage();
    pkt.clearPayloadOneof();

    final mutDouble = tm.getMutator<double>(1.0);
    mutDouble.setter(pkt, 5);
    expect(pkt.numberVal, 5.0);
    expect(mutDouble.getter(pkt), 5.0);

    final mutInt = tm.getMutator<int>(0);
    pkt = DataPackage();
    mutInt.setter(pkt, 99);
    expect(pkt.numberVal, 99);
    expect(mutInt.getter(pkt), 99);

    final mutProto = tm.getMutator<ExamplePayload>(ExamplePayload());
    final expPayload = ExamplePayload(name: "john", value: 42);
    pkt = DataPackage();
    mutProto.setter(pkt, expPayload);
    expect(pkt.blobVal.codec, Codec.CODEC_PROTO);
    expect(pkt.blobVal.messageType, 'examplemsg.ExamplePayload');

    final actualPayload = mutProto.getter(pkt);
    expect(actualPayload, expPayload);
    expect(actualPayload.name, 'john');
    expect(actualPayload.value, 42);
  });
}
