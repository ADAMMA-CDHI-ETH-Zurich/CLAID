
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
  
  static Future<void> start(final String socketPath, 
    final String configFilePath, final String hostId, 
    final String userId, final String deviceId, final ModuleFactory moduleFactory) async
  {
    // Creating the ModuleDispatcher loads the Middleware and starts the Claid core, as of now.
    _dispatcher = ModuleDispatcher(socketPath, configFilePath, hostId, userId, deviceId);


    final factories = moduleFactory.getFactories();

    await initModules(
        dispatcher: _dispatcher!,
        moduleFactories: factories);
  }

  static T? getModule<T extends Module>(final String moduleId)
  {
    Module? module = impl.ModuleManager.instance.getModule(moduleId);

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
