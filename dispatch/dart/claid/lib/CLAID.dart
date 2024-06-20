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


import 'package:claid/claid_core_bindings_generated.dart';
import 'package:claid/dispatcher.dart';
import 'package:claid/generated/claidservice.pb.dart';
import 'package:claid/middleware.dart';
import 'package:claid/module/module.dart';
import 'package:claid/module/module_factory.dart';
import 'package:claid/module/module_manager.dart';
import 'package:claid/RemoteFunction/RemoteFunctionHandler.dart';
import 'package:claid/RemoteFunction/RemoteFunction.dart';

import 'package:claid/logger/Logger.dart';

import './src/module_impl.dart' as impl;

class CLAID
{
  static ModuleDispatcher? _dispatcher;
  static MiddleWareBindings? _middleWare;
  static ModuleManager? _moduleManager;
  static ModuleFactory _moduleFactory = ModuleFactory();


  static Future<void> start(final String socketPath, 
    final String configFilePath, final String hostId, 
    final String userId, final String deviceId, {String libraryPath = ""}) async
  {
    // Creating the ModuleDispatcher loads the Middleware and starts the Claid core, as of now.

    if(libraryPath != "")
    {
      MiddleWareBindings.setLibraryPath(libraryPath);
    }
    _middleWare = _middleWare ?? MiddleWareBindings();
    _middleWare?.start(socketPath, configFilePath, hostId, userId, deviceId);

    if(_middleWare != null)
    {
      if(_middleWare?.attachCppRuntime()! != true)
      {
          Logger.logError("Failed to attach CLAID C++ Runtime.");
          return ;
      }

    }

    return attachDartRuntime(socketPath);
  }

  static Future<void> attachDartRuntime(final String socketPath) async
  {
    _dispatcher = ModuleDispatcher(socketPath);


    final factories = _moduleFactory.getFactories();

    _moduleManager = ModuleManager(_dispatcher!, factories);
    return _moduleManager!.start();
  }

  static T? getModule<T extends Module>(final String moduleId)
  {
    if(_moduleManager == null)
    {
      return null;
    }
    Module? module = _moduleManager!.getModule(moduleId);

    if(module == null)
    {
      return null;
    }

    if(module is T)
    {
      return module;
    }
    else
    {
      return null;
    }
  }

  static ModuleManager? getModuleManager()
  {
    return CLAID._moduleManager;
  }

  static RemoteFunctionHandler getRemoteFunctionHandler() 
  {
    return CLAID._moduleManager!.getRemoteFunctionHandler();
  }

  static Future<Map<String, String>?> getRunningModules() async
  {
    RemoteFunction<Map<String, String>>? mappedFunction = 
          getRemoteFunctionHandler().mapRuntimeFunction<Map<String, String>>(
                Runtime.MIDDLEWARE_CORE, "get_all_running_modules_of_all_runtimes",
                Map<String, String>(), []);

    return (mappedFunction!.execute([]))!;
  }

  static void registerModule<T extends Module>(String name, FactoryFunc factoryFunc) 
  {
    CLAID._moduleFactory.registerClass<T>(name, factoryFunc);
  }
}
