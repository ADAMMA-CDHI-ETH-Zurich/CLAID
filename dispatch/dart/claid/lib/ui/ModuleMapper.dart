import 'package:claid/CLAID.dart';
import 'package:claid/module.dart';

class ModuleMapper<ModuleType extends Module>
{
  // Id of the Module to Map
  String _moduleId;
  ModuleType? mappedModule;

  ModuleMapper({required String moduleId}) : _moduleId = moduleId
  {
    mappedModule = CLAID.getModule<ModuleType>(this._moduleId);
  }

  ModuleType? getModule()
  {
    return this.mappedModule;
  }

}