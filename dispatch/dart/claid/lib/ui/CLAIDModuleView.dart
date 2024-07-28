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

  RemoteFunction<T> mapMiddlewareFunction<T>(String functionName, T returnType, List<dynamic> parameters)
  {
    return remoteFunctionHandler!.mapRuntimeFunction(Runtime.MIDDLEWARE_CORE, functionName, returnType, parameters);
  }



  Future<bool?> subscribeModuleChannel<T>(String channelName, T dataType, DataReceiverCallback<T> callback) async
  {
    RemoteFunction<bool> function = mapMiddlewareFunction<bool>("add_loose_direct_subscription", false, [LooseDirectChannelSubscription()]);

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