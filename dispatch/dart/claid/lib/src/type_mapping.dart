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



import 'package:claid/generated/claidservice.pb.dart';
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

class TypeMapping {
  final _protoCodecMap = <String, ProtoCodec>{};

  ProtoCodec _getProtoCodec(GeneratedMessage msg) {
    final fullName = msg.info_.qualifiedMessageName;
    return _protoCodecMap[fullName] ?? ProtoCodec(msg);
  }

  Mutator<T> getMutator<T>(T inst) {
    if (inst is double) {
      return Mutator<T>(
          (p, v) => p.numberVal = v as double, (p) => p.numberVal as T);
    }

    if (inst is bool) {
      return Mutator<T>((p, v) => p.boolVal = v as bool, (p) => p.boolVal as T);
    }

    if (inst is String) {
      return Mutator<T>(
          (p, v) => p.stringVal = v as String, (p) => p.stringVal as T);
    }

    // List double
    if (inst is List<double>) {
      return Mutator<T>(
          (p, v) => p.numberArrayVal = NumberArray(val: v as List<double>),
          (p) => p.numberArrayVal.val as T);
    }

    // List string
    if (inst is List<String>) {
      return Mutator<T>(
          (p, v) => p.stringArrayVal = StringArray(val: v as List<String>),
          (p) => p.stringArrayVal.val as T);
    }

    // Map double
    if (inst is Map<String, double>) {
      return Mutator<T>(
          (p, v) => p.numberMap = NumberMap(val: v as Map<String, double>),
          (p) => p.numberMap.val as T);
    }

    // Map string
    if (inst is Map<String, String>) {
      return Mutator<T>(
          (p, v) => p.stringMap = StringMap(val: v as Map<String, String>),
          (p) => p.stringMap.val as T);
    }

    // Protobuf
    if (inst is GeneratedMessage) {
      final codec = _getProtoCodec(inst);
      return Mutator.wrap<T, GeneratedMessage>((p, v) {
        p.blobVal = codec.encode(v);
      }, (p) => codec.decode(p.blobVal));
    }

    throw ArgumentError('Type "${inst.runtimeType}" is not a valid type.');
  }
}
