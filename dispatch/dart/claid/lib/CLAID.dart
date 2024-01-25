
import 'package:claid/claid_core_bindings_generated.dart';
import 'package:claid/dispatcher.dart';
import 'package:claid/generated/claidservice.pb.dart';
import 'package:claid/middleware.dart';
import 'package:claid/module.dart';
import 'package:claid/module_factory.dart';
import './src/module_impl.dart' as impl;

class CLAID
{
  static ModuleDispatcher? _dispatcher;
  static MiddleWareBindings? _middleWare;
  static impl.ModuleManager? _moduleManager;

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

    _moduleManager = impl.ModuleManager(_dispatcher!, factories);
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
}
