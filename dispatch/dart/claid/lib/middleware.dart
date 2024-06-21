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



import 'dart:io';
import 'dart:async';
import 'dart:ffi' as ffi;
import 'dart:isolate'; // Import Isolate class

import 'package:ffi/ffi.dart';
import "package:path/path.dart" show dirname;
import 'claid_core_bindings_generated.dart';
import 'dart:io' show Platform;
const String _libName = 'claid_capi';
final String _platform = Platform.isAndroid ? 'android' : 'linux';

class MiddleWareBindings {
  var _coreHandle = ffi.Pointer<ffi.Void>.fromAddress(0);
  bool _ready = false;
  bool _wasMiddlewareStartedFromDart = false;
  static String _libraryPath = "";
  late ffi.DynamicLibrary _dyLib;
  /// The bindings to the native functions in [_dylib].
  late ClaidCoreBindings _bindings;
  
  MiddleWareBindings()
  {
    loadCLAIDLibrary();
  }

  // Starts the CLAID middleware.
  bool start(String socketPath, String configFile, String hostId,
      String userId, String deviceId) 
  {
    _coreHandle = _bindings.start_core(
        toCharPointer(socketPath),
        toCharPointer(configFile),
        toCharPointer(hostId),
        toCharPointer(userId),
        toCharPointer(deviceId));
    _ready = _coreHandle.address != 0;

    if(_ready)
    {
      _wasMiddlewareStartedFromDart = true;
    }

    return _ready;
  }

  bool attachCppRuntime()
  {
    return _bindings.attach_cpp_runtime(_coreHandle) != null;
  }

  void shutdown() {
    if (_ready && _wasMiddlewareStartedFromDart) {
      _bindings.shutdown_core(_coreHandle);
      _ready = false;
      _wasMiddlewareStartedFromDart = false;
    }
  }

  static void setLibraryPath(String path)
  {
    _libraryPath = path;
  }

  static String getLibraryPath(String path)
  {
    return _libraryPath;
  }


  /// The dynamic library in which the symbols for [ClaidCoreBindings] can be found.
  void loadCLAIDLibrary()
  {
    ffi.DynamicLibrary? library;
    if (Platform.isMacOS || Platform.isIOS)
    {

      String path = _libraryPath != "" ? _libraryPath : "../../dispatch/dart/claid/blobs/lib$_libName.dylib";
        print("LibraryPath $path");

      if(!File(path).existsSync())
      {
        path = "blobs/lib$_libName.dylib";
      }
      library = ffi.DynamicLibrary.open(path);
    }
    if (Platform.isLinux) 
    {
        String path = _libraryPath != "" ? _libraryPath : "../../dispatch/dart/claid/blobs/lib${_libName}_${_platform}.so";

      if(!File(path).existsSync())
      {
        path = "blobs/lib${_libName}_${_platform}.so";
      }
      library = ffi.DynamicLibrary.open(path);
    }
    if (Platform.isAndroid)
    {
      library = ffi.DynamicLibrary.open('libclaid_capi_android.so');
    }
    if (Platform.isWindows) 
    {
      library = ffi.DynamicLibrary.open('$_libName.dll');
    }

    if(library != null)
    {
      _dyLib = library;
      _bindings = ClaidCoreBindings(_dyLib);
      return;
    }
    throw UnsupportedError('Unknown platform: ${Platform.operatingSystem}');
  }


  bool get ready => _ready;
}



 




/// Helper function to get a char* pointer from a Dart string to pass
/// it to c function.
ffi.Pointer<ffi.Char> toCharPointer(String str) {
  return str.toNativeUtf8().cast<ffi.Char>();
}
