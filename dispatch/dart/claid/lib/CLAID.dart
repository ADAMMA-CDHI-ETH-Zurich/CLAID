
import 'package:claid/claid_core_bindings_generated.dart';
import 'package:claid/dispatcher.dart';
import 'package:claid/generated/claidservice.pb.dart';
import 'package:claid/middleware.dart';
import 'package:claid/module.dart';
import 'package:claid/module_factory.dart';

class CLAID
{
  static ModuleDispatcher? dispatcher;
  static Future<void> start(final String socketPath, 
    final String configFilePath, final String hostId, 
    final String userId, final String deviceId, final ModuleFactory moduleFactory) async
  {
    // Creating the ModuleDispatcher loads the Middleware and starts the Claid core, as of now.
    dispatcher = ModuleDispatcher(socketPath, configFilePath, hostId, userId, deviceId);


    final factories = moduleFactory.getFactories();

    await initModules(
        dispatcher: dispatcher!,
        moduleFactories: factories);
  }
}
