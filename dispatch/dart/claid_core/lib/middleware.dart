import 'dart:io';
import 'dart:ffi' as ffi;
import 'package:ffi/ffi.dart';

import 'claid_core_bindings_generated.dart';

const String _libName = 'claid_capi';
final String _platform = Platform.isAndroid ? 'android' : 'linux';

class MiddleWare {
  var _coreHandle = ffi.Pointer<ffi.Void>.fromAddress(0);
  var _ready = false;

  MiddleWare(String socketPath, configFile, userId, deviceId) {
    _coreHandle = _bindings.start_core(
        toCharPointer(socketPath),
        toCharPointer(configFile),
        toCharPointer(userId),
        toCharPointer(deviceId));
    _ready = _coreHandle.address != 0;
  }

  void shutdown() {
    if (_ready) {
      _bindings.shutdown_core(_coreHandle);
      _ready = false;
    }
  }

  bool get ready => _ready;
}

/// The dynamic library in which the symbols for [ClaidCoreBindings] can be found.
final ffi.DynamicLibrary _dylib = () {
  if (Platform.isMacOS || Platform.isIOS) {
    return ffi.DynamicLibrary.open('$_libName.framework/$_libName');
  }
  if (Platform.isAndroid || Platform.isLinux) {
    return ffi.DynamicLibrary.open('lib${_libName}_$_platform.so');
  }
  if (Platform.isWindows) {
    return ffi.DynamicLibrary.open('$_libName.dll');
  }
  throw UnsupportedError('Unknown platform: ${Platform.operatingSystem}');
}();

/// The bindings to the native functions in [_dylib].
final ClaidCoreBindings _bindings = ClaidCoreBindings(_dylib);

/// Helper function to get a char* pointer from a Dart string to pass
/// it to c function.
ffi.Pointer<ffi.Char> toCharPointer(String str) {
  return str.toNativeUtf8().cast<ffi.Char>();
}
