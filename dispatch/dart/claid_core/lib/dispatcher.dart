import 'dart:async';
import 'dart:io';

import 'package:claid_core/generated/claidservice.pbgrpc.dart';
import 'package:grpc/grpc.dart';

import 'middleware.dart';

class StreamingError implements Error {
  final String message;
  final bool cancel;
  const StreamingError(this.message, this.cancel);

  @override
  StackTrace? get stackTrace => StackTrace.empty;
}

class ModDescriptor {
  final String moduleId;
  final String moduleClass;
  final Map<String, String> properties;
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
  final String _socketPath;
  late ClaidServiceClient _stub;
  late ClientChannel _channel;
  ResponseStream<DataPackage>? _responseStream;
  StreamController<DataPackage>? _outputController;

  ModuleDispatcher(this._socketPath, String configFile, String hostId,
      String userId, String deviceId)
      : _middleWare = MiddleWareBindings(
            _socketPath, configFile, hostId, userId, deviceId) {
    _channel = ClientChannel(
        InternetAddress(_socketPath, type: InternetAddressType.unix),
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

    final listenCalled = Completer<void>();

    final listenFunc = outputController.onListen;
    outputController.onListen = () async {
      listenFunc?.call();
      await Future.delayed(Duration.zero, () {
        listenCalled.complete();
      });
    };

    final pingReceived = Completer<void>();
    _responseStream = _stub.sendReceivePackages(outputController.stream);

    final ret = _wrapInputStream(_responseStream!, pingReceived);

    await listenCalled.future;

    // Send the ping to the server. The return ping is caught in wrapInputStream.
    final pingPkt = DataPackage()
      ..controlVal = ControlPackage(
          ctrlType: CtrlType.CTRL_RUNTIME_PING, runtime: Runtime.RUNTIME_DART);
    _outputController = outputController;
    outputController.add(pingPkt);
    return ret;
  }

  Future<void> closeRuntime() async {
    // await _outputController?.close();
    // await Future.delayed(const Duration(seconds: 5));
    // await _responseStream?.cancel();
  }

  // Given a the order by which the different modules are initialized, the
  // dispatcher can re-arrange the order.
  // This is only used by the mock dispatcher to piroritize modules under
  // test over mock modules.
  List<String> getModuleOrder(List<String> proposedModuleOrder) {
    return proposedModuleOrder;
  }

  Stream<DataPackage> _wrapInputStream(
      Stream<DataPackage> source, Completer<void> completer) async* {
    var first = false;

    await for (var pkt in source) {
      // Always deal with errors first.
      if (pkt.hasControlVal() &&
          pkt.controlVal.ctrlType == CtrlType.CTRL_ERROR) {
        final ctrlMsg = pkt.controlVal;
        final err = ctrlMsg.errorMsg;
        if (err.cancel) {
          throw StreamingError(err.message, err.cancel);
        }
        // TODO: convert to log message
        print('Got error: ${err.message}. Continuing.');
        continue;
      }

      if (!first) {
        first = true;
        completer.complete();

        if (pkt.controlVal.ctrlType != CtrlType.CTRL_RUNTIME_PING) {
          throw const StreamingError(
              'Excpected control packages but did not receive it.', true);
        }
        continue;
      }

      // TODO: Here we could filter control packets and not pass them
      // on to the business logic.
      yield pkt;
    }
  }
}
