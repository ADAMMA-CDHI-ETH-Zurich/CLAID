import 'dart:io';

import 'package:flutter/material.dart';

import 'SelectableDeviceViewWidget.dart';
import 'package:claid/generated/claidservice.pb.dart';
import 'package:claid/RemoteFunction/RemoteFunction.dart';
import 'package:claid/RemoteFunction/RemoteFunctionHandler.dart';

import 'package:claid/ui/DataReceiver.dart';
import 'package:claid/module/module_manager.dart';
import 'package:claid/module/type_mapping.dart';
import 'package:claid/logger/Logger.dart';

abstract class CLAIDModuleView extends StatefulWidget
{
  final String entityName; // Name of this CLAIDModuleView
  final String mappedModuleId;
  final String moduleClass;
  late RemoteFunctionHandler? remoteFunctionHandler;

  Map<String, DataReceiver> _looseDirectSubscriptionCallbacks = {};

  CLAIDModuleView(this.entityName, this.mappedModuleId,
    this.moduleClass, ModuleManager moduleManager, {Key? key}) : super(key: key)
  {
    print("initstate CLAIDModuleView constr ${mappedModuleId.length} $mappedModuleId $moduleClass");
    this.remoteFunctionHandler = moduleManager.getRemoteFunctionHandler();
    moduleManager.registerDataReceiverEntity(entityName, this.onDataFromLooseDirectSubscription);
  }

  RemoteFunction<T> mapFunction<T>(String functionName, T returnType, List<dynamic> parameters)
  {
    return remoteFunctionHandler!.mapModuleFunction(mappedModuleId, functionName, returnType, parameters);
  }

  RemoteFunctionWithoutParameter<T> mapModuleFunctionWithoutParameter<T>(String functionName, T returnType)
  {
    return remoteFunctionHandler!.mapModuleFunctionWithoutParameter(mappedModuleId, functionName, returnType);
  }

  RemoteFunctionWith1Parameter<T> mapModuleFunctionWith1Parameter<T>(String functionName, T returnType, dynamic parameter1)
  {
    return remoteFunctionHandler!.mapModuleFunctionWith1Parameter(mappedModuleId, functionName, returnType, parameter1);
  }

  RemoteFunctionWith2Parameters<T> mapModuleFunctionWith2Parameters<T>(String functionName, T returnType, dynamic parameter1, dynamic parameter2)
  {
    return remoteFunctionHandler!.mapModuleFunctionWith2Parameters(
        mappedModuleId, functionName, returnType, parameter1, parameter2);
  }

  RemoteFunctionWith3Parameters<T> mapModuleFunctionWith3Parameters<T>(String functionName, T returnType, dynamic parameter1, dynamic parameter2, dynamic parameter3)
  {
    return remoteFunctionHandler!.mapModuleFunctionWith3Parameters(
        mappedModuleId, functionName, returnType, parameter1, parameter2, parameter3);
  }

  RemoteFunctionWith4Parameters<T> mapModuleFunctionWith4Parameters<T>(
    String functionName, T returnType, dynamic parameter1, dynamic parameter2, dynamic parameter3, dynamic parameter4) 
  {
    return remoteFunctionHandler!.mapModuleFunctionWith4Parameters(
        mappedModuleId, functionName, returnType, parameter1, parameter2, parameter3, parameter4);
  }

  RemoteFunctionWith5Parameters<T> mapModuleFunctionWith5Parameters<T>(
    String functionName, T returnType, dynamic parameter1, dynamic parameter2, dynamic parameter3, dynamic parameter4, dynamic parameter5) 
  {
    return remoteFunctionHandler!.mapModuleFunctionWith5Parameters(
        mappedModuleId, functionName, returnType, parameter1, parameter2, parameter3, parameter4, parameter5);
  }

  RemoteFunctionWith6Parameters<T> mapModuleFunctionWith6Parameters<T>(
      String functionName, T returnType, dynamic parameter1, dynamic parameter2, dynamic parameter3, 
      dynamic parameter4, dynamic parameter5, dynamic parameter6) 
  {
    return remoteFunctionHandler!.mapModuleFunctionWith6Parameters(
        mappedModuleId, functionName, returnType, parameter1, parameter2, parameter3, parameter4, parameter5, parameter6);
  }

  RemoteFunction<T> mapMiddlewareFunction<T>(String functionName, T returnType, List<dynamic> parameters)
  {
    return remoteFunctionHandler!.mapRuntimeFunction(Runtime.MIDDLEWARE_CORE, functionName, returnType, parameters);
  }

  Future<bool?> subscribeModuleChannel<T>(String channelName, T dataType, DataReceiverCallback<T> callback) async
  {
    RemoteFunction<bool> function = mapMiddlewareFunction<bool>("add_loose_direct_subscription_if_not_exists", false, [LooseDirectChannelSubscription()]);

    LooseDirectChannelSubscription subscription = LooseDirectChannelSubscription();
    subscription.subscribedModule = mappedModuleId;
    subscription.subscribedChannel = channelName;
    subscription.subscriberRuntime = Runtime.RUNTIME_DART;
    subscription.subscriberEntity = entityName;

    String id = _makeLooseDirectSubscriptionId(subscription);
    Mutator<T> mutator = TypeMapping().getMutator(dataType);
    DataReceiver<T> receiver = DataReceiver<T>(mutator, callback);

    _looseDirectSubscriptionCallbacks[id] = receiver;

    return function.executeWithParameters([subscription]);
  }

  String _makeLooseDirectSubscriptionId(LooseDirectChannelSubscription subscription)
  {
    return "${entityName}:${subscription.subscribedModule}:${subscription.subscribedChannel}";
  }

  void onDataFromLooseDirectSubscription(DataPackage package)
  {
      LooseDirectChannelSubscription subscription = package.controlVal.looseDirectSubscription;
      String id = _makeLooseDirectSubscriptionId(subscription);

      if(!_looseDirectSubscriptionCallbacks.containsKey(id))
      {
        Logger.logError("Failed to forward data from loose direct connection. " 
        + "Entity $entityName does not have DataReceiver for subscription \"$id\"");
        return;
      }

      _looseDirectSubscriptionCallbacks[id]!.onData(package);
  }

  String getName() => mappedModuleId;
  Widget getSubCaptionWidget();
  Widget getImage(BuildContext context);
  String getModuleType();
  String getModuleSubType();

  // Called when the ModuleView is selected from the ModuleListView.
  void onShown()
  {
    print("ModuleView shown!");
  }

  // Called when changing to another ModuleView as selected from the ModuleListView.
  void onHidden()
  {
    print("ModuleView hidden!");

  }

}