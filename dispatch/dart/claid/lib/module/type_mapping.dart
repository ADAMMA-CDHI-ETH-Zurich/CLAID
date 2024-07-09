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
import 'package:fixnum/src/int64.dart';
import 'dart:ffi';

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

  DataPackage _setProtoPayload<T>(DataPackage package, GeneratedMessage payload)
  {
    ProtoCodec codec = _getProtoCodec(payload);
    Blob blob = codec.encode(payload);
    package.payload = blob;
    return package;
  }

  GeneratedMessage _getProtoPayload(DataPackage package, GeneratedMessage exampleInstance)
  {
    ProtoCodec codec = _getProtoCodec(exampleInstance);
    GeneratedMessage msg = codec.decode(package.payload);
    
    return msg;
  }


  Mutator<T> getMutator<T>(T inst) 
  {
  if (inst is Int64) {
      return Mutator<T>(
          (p, v) => _setProtoPayload(p, IntVal()..val = v as Int64),
          (p) => (_getProtoPayload(p, IntVal()) as IntVal).val as T);
    }

    if (inst is int) {
      return Mutator<T>(
          (p, v) => _setProtoPayload(p, IntVal()..val = Int64(v as int)),
          (p) => (_getProtoPayload(p, IntVal()) as IntVal).val.toInt() as T);
    }

    if (inst is double) {
      return Mutator<T>(
          (p, v) => _setProtoPayload(p, DoubleVal()..val = v as double), 
          (p) => (_getProtoPayload(p, DoubleVal()) as DoubleVal).val as T);
    }


    if (inst is bool) 
    {
      return Mutator<T>(
          (p, v) => _setProtoPayload(p, BoolVal()..val = v as bool), 
          (p) => (_getProtoPayload(p, BoolVal()) as BoolVal).val as T);
    }

    if (inst is String) {
      return Mutator<T>(
          (p, v) => _setProtoPayload(p, StringVal()..val = v as String), 
          (p) => (_getProtoPayload(p, StringVal()) as StringVal).val as T);
    }

    // List double
    if (inst is List<double>) {
      return Mutator<T>(
          (p, v) => _setProtoPayload(p, NumberArray(val: v as List<double>)),
          (p) => (_getProtoPayload(p, NumberArray()) as NumberArray).val as T);
    }

    // List string
    if (inst is List<String>) {
      return Mutator<T>(
          (p, v) => _setProtoPayload(p, StringArray(val: v as List<String>)),
          (p) => (_getProtoPayload(p, StringArray()) as StringArray).val as T);
    }

    // Map double
    if (inst is Map<String, double>) {
      return Mutator<T>(
        (p, v) => _setProtoPayload(p, NumberMap(val: v as Map<String, double>)),
        (p) => (_getProtoPayload(p, NumberMap()) as NumberMap).val as T);
    }

    // Map string
    if (inst is Map<String, String>) {

      return Mutator<T>(
        (p, v) => _setProtoPayload(p, StringMap(val: v as Map<String, String>)),
        (p) => (_getProtoPayload(p, StringMap()) as StringMap).val as T);


    }

    // Protobuf
    if (inst is GeneratedMessage) {
      final codec = _getProtoCodec(inst);
      return Mutator.wrap<T, GeneratedMessage>((p, v) {
        p.payload = codec.encode(v);
      }, (p) => codec.decode(p.payload));
    }

    if(T == null)
    {
      return Mutator<T>(
        (p, v) => {},
        (p) => null as T);
    }

    throw ArgumentError('Type "${inst.runtimeType}" is not a valid type to use with CLAID. Type is not supported.');
  }

    Type _getType<T>() => T;

}
