import 'package:claid/RemoteFunction/RemoteFunctionHandler.dart';
import 'package:claid/ui/CLAIDModuleView.dart';
import 'package:claid/ui/EmptyDefaultDeviceView.dart';

typedef ViewFactoryFunc = CLAIDModuleView Function(String moduleId, String moduleClass, RemoteFunctionHandler handler);

class CLAIDModuleViewToClassMap
{
  CLAIDModuleViewToClassMap()
  {

  }

  static Map<String, ViewFactoryFunc> _lookupMap = Map<String, ViewFactoryFunc>();

  static registerModuleClass(String moduleClass, ViewFactoryFunc factoryFunction)
  {
    _lookupMap[moduleClass] = factoryFunction;
  }

  CLAIDModuleView getView(String moduleId, String moduleClass, RemoteFunctionHandler remoteFunctionHandler)
  {
    ViewFactoryFunc? func = _lookupMap[moduleClass];
    if(func != null)
    {
     return func!(moduleId, moduleClass, remoteFunctionHandler);
    }
    return EmptyDefaultDeviceView(moduleId, moduleClass, remoteFunctionHandler);
  }

  static Map<String, ViewFactoryFunc> getMap() => _lookupMap;
}