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
import 'package:claid/logger/Logger.dart';
import 'package:claid/RemoteFunction/RemoteFunctionHandler.dart';
import 'package:claid/RemoteFunction/RemoteFunction.dart';
import 'package:claid/ui/CLAIDModuleViewToClassMap.dart';

import 'package:path_provider/path_provider.dart';
import 'package:claid/logger/Logger.dart';

import 'package:claid/package/CLAIDPackage.dart';
import 'package:claid/package/CLAIDPackageLoader.dart';


import './src/module_impl.dart' as impl;

import 'package:flutter/material.dart';
import 'package:path_provider/path_provider.dart';
import 'package:flutter/services.dart';
import 'dart:io';

class CLAIDSpecialPermissionsConfig
{
  String _configIdentifier;

  CLAIDSpecialPermissionsConfig._(this._configIdentifier)
  {

  }

  static CLAIDSpecialPermissionsConfig almightyCLAID()
  {
    return CLAIDSpecialPermissionsConfig._("almightyCLAID");

  }

  static CLAIDSpecialPermissionsConfig regularConfig()
  {
    return CLAIDSpecialPermissionsConfig._("regularConfig");
  }

  static allStorageAccessConfig()
  {
    return CLAIDSpecialPermissionsConfig._("allStorageAccessConfig");
  }

  String getIdentifier()
  {
    return this._configIdentifier;
  }

}

class CLAIDPersistanceConfig
{
  String _configIdentifier;
  
  CLAIDPersistanceConfig._(this._configIdentifier)
  {

  }

  static CLAIDPersistanceConfig maximumPersistance()
  {
    return CLAIDPersistanceConfig._("maximumPersistance");
  }

  static CLAIDPersistanceConfig minimumPersistance()
  {
    return CLAIDPersistanceConfig._("minimumPersistance");
  }

  String getIdentifier()
  {
    return this._configIdentifier;
  }
}

class CLAID
{
  static ModuleDispatcher? _dispatcher;
  static MiddleWareBindings? _middleWare;
  static ModuleManager? _moduleManager;
  static ModuleFactory _moduleFactory = ModuleFactory();
  static MethodChannel javaChannel = MethodChannel('adamma.c4dhi.claid/FLUTTERCLAID');

  static Future<bool> startInForeground(
    final String configFilePath, final String hostId, 
    final String userId, final String deviceId, 
    CLAIDSpecialPermissionsConfig specialPermissionsConfig,
    {List<CLAIDPackage>? claidPackages = null},
  ) async
  {
    Logger.logInfo("CLAID startInForeground called");
    await loadPackages(claidPackages);
    Directory? appDocDir = await getApplicationDocumentsDirectory();

    // Construct the path to the Android/media directory
    String mediaDirectoryPath = '${appDocDir!.path}';

    mediaDirectoryPath = mediaDirectoryPath.replaceAll("app_flutter", "files");
    String socketPath = "unix://" + mediaDirectoryPath + "/" + "claid_local.grpc";
    print("Calling platform function");
    bool result = await _startCLAIDInForegroundFromNativeLanguage(socketPath,
        configFilePath, hostId, userId, deviceId, specialPermissionsConfig);

    if(!result)
    {
      Logger.logFatal("Failed to start CLAID in foreground");
      return false;
    }
    await attachDartRuntime(socketPath);
    return true;
  }

  static Future<bool> startInBackground(
    final String configFilePath, final String hostId, 
    final String userId, final String deviceId, 
    CLAIDSpecialPermissionsConfig specialPermissionsConfig,
    CLAIDPersistanceConfig persistanceConfig,
    {List<CLAIDPackage>? claidPackages = null},
  ) async
  {
    Logger.logInfo("CLAID startInBackground called");
    await loadPackages(claidPackages);

    Directory? appDocDir = await getApplicationDocumentsDirectory();

    // Construct the path to the Android/media directory
    String mediaDirectoryPath = '${appDocDir!.path}';

    mediaDirectoryPath = mediaDirectoryPath.replaceAll("app_flutter", "files");
    String socketPath = "unix://" + mediaDirectoryPath + "/" + "claid_local.grpc";
    bool result = await _startCLAIDInBackgroundFromNativeLanguage(socketPath, configFilePath,
      hostId, userId, deviceId, specialPermissionsConfig, persistanceConfig);

    if(!result)
    {
      Logger.logFatal("Failed to start CLAID in background");
      return false;
    }
    await attachDartRuntime(socketPath);
    return true;
  }

  static Future<bool> startMiddleware(final String socketPath, 
    final String configFilePath, final String hostId, 
    final String userId, final String deviceId, 
    CLAIDSpecialPermissionsConfig specialPermissionsConfig, {String libraryPath = ""}) async
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
          Logger.logFatal("Failed to attach CLAID C++ Runtime.");
          return false;
      }

    }

    await attachDartRuntime(socketPath);
    return true;
  }

  static Future<void> attachDartRuntime(final String socketPath) async
  {
    Logger.logInfo("Attach runtime");
    _dispatcher = ModuleDispatcher(socketPath);


    final factories = _moduleFactory.getFactories();

    _moduleManager = ModuleManager(_dispatcher!, factories);
    return _moduleManager!.start();
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

  // Let a native language (e.g., Java on Android or Objective-C on iOS) start CLAID.
  static Future<bool> _startCLAIDInForegroundFromNativeLanguage(
        String socketPath, String configFilePath, String hostId, 
        String userId, String deviceId,
      CLAIDSpecialPermissionsConfig specialPermissionsConfig) async
  {
    try 
    {
      List<String> arguments = [socketPath, configFilePath, hostId, userId, deviceId, specialPermissionsConfig.getIdentifier()]; // Example list of strings
      print("Waiting for platform functioN");
      return await javaChannel.invokeMethod('startInForeground', arguments);
      
    } on PlatformException catch (e) {
      // Handle exception
      Logger.logFatal('CLAID platform error: ${e.message}');
      return false;
    }
  }

  // Let a native language (e.g., Java on Android or Objective-C on iOS) start CLAID.
  static Future<bool> _startCLAIDInBackgroundFromNativeLanguage(
        String socketPath, String configFilePath, String hostId, 
        String userId, String deviceId, 
        CLAIDSpecialPermissionsConfig specialPermissionsConfig,
        CLAIDPersistanceConfig persistanceConfig) async 
  {
    try 
    {
      List<String> arguments = [socketPath, configFilePath, hostId, 
        userId, deviceId, specialPermissionsConfig.getIdentifier(), persistanceConfig.getIdentifier()]; // Example list of strings

      return await javaChannel.invokeMethod('startInBackground', arguments);
      
    } on PlatformException catch (e) {
      // Handle exception
      Logger.logFatal('Error: ${e.message}');
      return false;
    }
  }

  static void registerViewClassForModule(String moduleClass, ViewFactoryFunc factoryFunc)
  {
    CLAIDModuleViewToClassMap.registerModuleClass(moduleClass, factoryFunc);
  }

  static void loadPackages(List<CLAIDPackage>? packages) async
  {
    if(packages != null)
    {
      for(CLAIDPackage package in packages!)
      {
        CLAIDPackageLoader.loadPackage(package);
      }
    }
  }
}
