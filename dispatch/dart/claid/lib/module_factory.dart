import 'package:claid/module.dart';


class ModuleFactory {
  var _factories = <String, FactoryFunc> {};

  void registerClass<T extends Module>(String name, FactoryFunc factoryFunc) 
  {
    _factories[name] = factoryFunc;
  }

  T getInstance<T extends Module>(String name) 
  {
    if (_factories.containsKey(name)) 
    {
      return _factories[name]!() as T;
    } 
    else 
    {
      throw Exception("Class with name '$name' not registered");
    }
  }

  Map<String, FactoryFunc> getFactories()
  {
    return _factories;
  }
}