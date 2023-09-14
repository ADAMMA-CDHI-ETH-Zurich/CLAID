import 'dart:async';
import 'dart:io';

import 'dart:async';
import 'dart:io';

import 'package:claid_core/generated/claidservice.pbgrpc.dart';
import 'package:grpc/grpc.dart';

import 'middleware.dart';

class DispatcherClient {
  final MiddleWare _middleWare;
  final String socketPath;
  late ClaidServiceClient _stub;
  late ClientChannel _channel;

  DispatcherClient(
      this.socketPath, String configFile, String userId, String deviceId)
      : _middleWare = MiddleWare(socketPath, configFile, userId, deviceId) {
    _channel = ClientChannel(socketPath,
        options:
            const ChannelOptions(credentials: ChannelCredentials.insecure()));
    _stub = ClaidServiceClient(_channel,
        options: CallOptions(timeout: const Duration(seconds: 30)));
  }

  bool start() {
    if (!_middleWare.ready) {
      return false;
    }

    return true;
  }

  void shutdown() {
    _middleWare.shutdown();
  }

  Future<ModuleListResponse> getModuleList(List<String> moduleClasses) async {
    final req = ModuleListRequest(
        runtime: Runtime.RUNTIME_DART, supportedModuleClasses: moduleClasses);
    return await _stub.getModuleList(req);
  }

  Future<void> initRuntime(Map<String, List<DataPackage>> modules) async {
    final req = InitRuntimeRequest(runtime: Runtime.RUNTIME_DART);
    await _stub.initRuntime(req);
  }
}
