import 'package:claid/RemoteFunction/RemoteFunctionHandler.dart';
import 'package:claid/module/module_manager.dart';

import 'package:claid/ui/CLAIDModuleView.dart';
import 'package:claid/ui/EmptyDefaultDeviceView.dart';

typedef ViewFactoryFunc = CLAIDModuleView Function(String entityName, String moduleId, String moduleClass, ModuleManager moduleManager);

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

  CLAIDModuleView getView(String moduleId, String moduleClass, ModuleManager moduleManager)
  {
    String entityName = "CLAID_DART_${moduleId}_ModuleView";
    ViewFactoryFunc? func = _lookupMap[moduleClass];
    if(func != null)
    {
     return func!(entityName, moduleId, moduleClass, moduleManager);
    }
    return EmptyDefaultDeviceView(entityName, moduleId, moduleClass, moduleManager);
  }

  static Map<String, ViewFactoryFunc> getMap() => _lookupMap;
}