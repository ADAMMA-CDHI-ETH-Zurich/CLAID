import 'dart:async';
import 'dart:ffi';
import 'dart:io';
import 'dart:isolate';

import 'claid_core_bindings_generated.dart';
import 'dart:ffi' as ffi;

const String _libName = 'claid_capi';

// The pointer to the CLAID middleware subsystem.
ffi.Pointer<ffi.Void>? _coreHandle;

bool initClaidCore(String socket, configFile, userId, deviceId) {
  _coreHandle =
      _bindings.start_core(socket_path, config_file, user_id, device_id);
  return false;
}

/// The dynamic library in which the symbols for [ClaidCoreBindings] can be found.
final DynamicLibrary _dylib = () {
  if (Platform.isMacOS || Platform.isIOS) {
    return DynamicLibrary.open('$_libName.framework/$_libName');
  }
  if (Platform.isAndroid || Platform.isLinux) {
    return DynamicLibrary.open('lib$_libName.so');
  }
  if (Platform.isWindows) {
    return DynamicLibrary.open('$_libName.dll');
  }
  throw UnsupportedError('Unknown platform: ${Platform.operatingSystem}');
}();

/// The bindings to the native functions in [_dylib].
final ClaidCoreBindings _bindings = ClaidCoreBindings(_dylib);
