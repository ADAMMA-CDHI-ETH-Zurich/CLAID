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

import './src/module_impl.dart' as impl;

class CLAID
{
  static ModuleDispatcher? _dispatcher;
  static MiddleWareBindings? _middleWare;
  static ModuleManager? _moduleManager;

  static void start(final String socketPath, 
    final String configFilePath, final String hostId, 
    final String userId, final String deviceId, final ModuleFactory moduleFactory)
  {
    // Creating the ModuleDispatcher loads the Middleware and starts the Claid core, as of now.

    _middleWare = _middleWare ?? MiddleWareBindings();
    _middleWare?.start(socketPath, configFilePath, hostId, userId, deviceId);

    attachDartRuntime(socketPath, moduleFactory);
  }

  static void attachDartRuntime(final String socketPath, final ModuleFactory moduleFactory)
  {
    _dispatcher = ModuleDispatcher(socketPath);


    final factories = moduleFactory.getFactories();

    _moduleManager = ModuleManager(_dispatcher!, factories);
    _moduleManager!.start();
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
}
