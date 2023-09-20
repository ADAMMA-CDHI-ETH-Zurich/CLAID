import 'package:claid_core/generated/claidservice.pb.dart';
import 'package:protobuf/protobuf.dart';

typedef SetterFn<T> = void Function(DataPackage pkt, T val);
typedef GetterFn<T> = T Function(DataPackage pkt);

class Mutator<T> {
  final SetterFn<T> setter;
  final GetterFn<T> getter;
  Mutator(this.setter, this.getter);

  // Helper function that wrappes the canonical type with the
  // specific type.
  static Mutator<T> wrap<T, V>(SetterFn<V> s, GetterFn<V> g) =>
      Mutator<T>((p, val) {
        s(p, val as V);
      }, (p) => g(p) as T);
}

typedef ProtoEncoder = Blob Function(GeneratedMessage);
typedef ProtoDecoder = GeneratedMessage Function(Blob);

class ProtoCodec {
  final String fullName;
  final GeneratedMessage _msg;
  late ProtoEncoder encode;
  late ProtoDecoder decode;
  ProtoCodec(this._msg) : fullName = _msg.info_.qualifiedMessageName {
    encode = _buildEncoder();
    decode = _buildDecoder();
  }

  ProtoEncoder _buildEncoder() {
    return (GeneratedMessage msg) => Blob(
        codec: Codec.CODEC_PROTO,
        payload: msg.writeToBuffer().toList(),
        messageType: fullName);
  }

  ProtoDecoder _buildDecoder() {
    return (Blob blob) =>
        _msg.info_.createEmptyInstance!()..mergeFromBuffer(blob.payload);
  }
}

final doubleMut =
    Mutator<double>((p, v) => p.numberVal = v, (p) => p.numberVal);

// final stringMut = Mutator<double>((p, v) { p.numberVal = v; }, (p) => p.numberVal);
// final doubleMut = Mutator<double>((p, v) { p.numberVal = v; }, (p) => p.numberVal);
// final doubleMut = Mutator<double>((p, v) { p.numberVal = v; }, (p) => p.numberVal);
// final doubleMut = Mutator<double>((p, v) { p.numberVal = v; }, (p) => p.numberVal);

class TypeMapping {
  final _protoCodecMap = <String, ProtoCodec>{};

  ProtoCodec _getProtoCodec(GeneratedMessage msg) {
    final fullName = msg.info_.qualifiedMessageName;
    return _protoCodecMap[fullName] ?? ProtoCodec(msg);
  }

  Mutator<T> getMutator<T>(T inst) {
    if (inst is int) {
      return Mutator<T>((pkt, val) => pkt.numberVal = (val as int).toDouble(),
          (pkt) => pkt.numberVal.toInt() as T);
    }

    //   , double>((pkt, val) => pkt.numberVal = ,)
    //   return Mutator<T>(
    //       (p, v) => p.numberVal = v as double, (p) => p.numberVal as T);
    // }

    if (T == double) {
      return Mutator<T>(
          (p, v) => p.numberVal = v as double, (p) => p.numberVal as T);
    }

    if (T == String) {
      return Mutator.wrap<T, String>((p, v) {
        p.stringVal = v;
      }, (p) => p.stringVal);
    }

    if (T == List<double>) {
      return Mutator.wrap<T, List<double>>((p, v) {
        p.numberArrayVal = NumberArray(val: v);
      }, (p) => p.numberArrayVal.val);
    }

    if (T == List<String>) {
      return Mutator.wrap<T, List<String>>((p, v) {
        p.stringArrayVal = StringArray(val: v);
      }, (p) => p.stringArrayVal.val);
    }

    if (T == Map<String, double>) {
      return Mutator.wrap<T, Map<String, double>>((p, v) {
        p.numberMap = NumberMap(val: v);
      }, (p) => p.numberMap.val);
    }

    if (T == Map<String, String>) {
      return Mutator.wrap<T, Map<String, String>>((p, v) {
        p.stringMap = StringMap(val: v);
      }, (p) => p.stringMap.val);
    }

    // if (tType <= const TypeHelper<Child>()) ...

    if (inst is GeneratedMessage) {
      final codec = _getProtoCodec(inst);
      return Mutator.wrap<T, GeneratedMessage>((p, v) {
        p.blobVal = codec.encode(v);
      }, (p) => codec.decode(p.blobVal));
    }

    throw AssertionError("unknown channel type");
  }
}

// bool isSubtype<S, T>() {
//   final x = <S>[];
//   return (x is List<T>);
// }

// class TypeHelper<T> {
//   const TypeHelper();
//   bool operator >=(TypeHelper other) => other is TypeHelper<T>;
//   bool operator <=(TypeHelper other) => other >= this;
//   bool operator >(TypeHelper other) => this >= other && !(other >= this);
//   bool operator <(TypeHelper other) => other >= this && !(this >= other);
// }
