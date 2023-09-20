import 'dart:async';
import 'dart:io';

import 'package:claid_core/generated/claidservice.pbgrpc.dart';
import 'package:grpc/grpc.dart';

import 'middleware.dart';

class ModDescriptor {
  final String moduleId;
  final String moduleClass;
  final Map<String, String> properties;
  // final List<ChanDesciptor> channels;
  const ModDescriptor(this.moduleId, this.moduleClass, this.properties);
}

// class ChanDesciptor {
//   final String channelId;
//   final String source;
//   final String target;
//   const ChanDesciptor(this.channelId, this.source, this.target);
// }

class ModuleDispatcher {
  final MiddleWareBindings _middleWare;
  final String socketPath;
  late ClaidServiceClient _stub;
  late ClientChannel _channel;

  ModuleDispatcher(this.socketPath, String configFile, String hostId,
      String userId, String deviceId)
      : _middleWare = MiddleWareBindings(
            socketPath, configFile, hostId, userId, deviceId) {
    _channel = ClientChannel(
        InternetAddress(socketPath, type: InternetAddressType.unix),
        options:
            const ChannelOptions(credentials: ChannelCredentials.insecure()));
    _stub = ClaidServiceClient(_channel);
    // options: CallOptions(timeout: const Duration(seconds: 30)));
  }

  bool start() => _middleWare.ready;

  void shutdown() {
    _middleWare.shutdown();
  }

  Future<List<ModDescriptor>> getModuleList(List<String> moduleClasses) async {
    final req = ModuleListRequest(
        runtime: Runtime.RUNTIME_DART, supportedModuleClasses: moduleClasses);
    final resp = await _stub.getModuleList(req);
    return resp.descriptors
        .map<ModDescriptor>(
            (e) => ModDescriptor(e.moduleId, e.moduleClass, e.properties))
        .toList();
  }

  Future<Stream<DataPackage>> initRuntime(
      Map<String, List<DataPackage>> modules,
      StreamController<DataPackage> outputController) async {
    final req = InitRuntimeRequest(
        runtime: Runtime.RUNTIME_DART,
        modules: modules.entries.map((e) => InitRuntimeRequest_ModuleChannels(
            moduleId: e.key, channelPackets: e.value)));
    await _stub.initRuntime(req);

    final inputStream = _stub.sendReceivePackages(outputController.stream);
    final ret = wrapInputStream(inputStream);

    // Send the ping to the server. The return ping is caught in wrapInputStream.
    outputController.add(DataPackage()
      ..controlVal = ControlPackage(ctrlType: CtrlType.CTRL_RUNTIME_PING));

    return ret;
  }

  Stream<DataPackage> wrapInputStream(Stream<DataPackage> source) async* {
    var first = false;
    await for (final pkt in source) {
      if (!first) {
        first = true;
        if (pkt.controlVal.ctrlType != CtrlType.CTRL_RUNTIME_PING) {
          throw AssertionError(
              'Excpected control packages but did not receive it.');
        }
        // Now we are good - the first package is absorbed.
      } else {
        // TODO: Here we could filter control packets and not pass them
        // on to the business logic.
        yield pkt;
      }
    }
  }
}
