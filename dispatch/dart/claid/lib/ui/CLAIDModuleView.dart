import 'dart:io';

import 'package:flutter/material.dart';

import 'SelectableDeviceViewWidget.dart';
import 'package:claid/generated/claidservice.pb.dart';
import 'package:claid/RemoteFunction/RemoteFunction.dart';
import 'package:claid/RemoteFunction/RemoteFunctionHandler.dart';

abstract class CLAIDModuleView extends StatefulWidget
{
  final String mappedModuleId;
  final String moduleClass;
  final RemoteFunctionHandler? remoteFunctionHandler;

  CLAIDModuleView(this.mappedModuleId,
    this.moduleClass, this.remoteFunctionHandler, {Key? key}) : super(key: key)
  {
    print("initstate CLAIDModuleView constr ${mappedModuleId.length} $mappedModuleId $moduleClass");
  }



  RemoteFunction<T> mapFunction<T>(String functionName, T returnType, List<dynamic> parameters)
  {
    return remoteFunctionHandler!.mapModuleFunction(mappedModuleId, functionName, returnType, parameters);
  }

  RemoteFunction<T> mapMiddlewareFunction<T>(String functionName, T returnType, List<dynamic> parameters)
  {
    return remoteFunctionHandler!.mapRuntimeFunction(Runtime.RUNTIME_CPP, functionName, returnType, parameters);
  }

  /*Future<bool> subscribeChannel<T>(String channelName, T dataType, ChannelCallback<T> callback) async
  {
    RemoteFunction<bool> function = mapFunction<bool>("soft_subscribe_data", bool(), [String()]);

    return function.execute(channelName);
  }*/

  String getName() => mappedModuleId;
  Widget getSubCaptionWidget();
  Widget getImage(BuildContext context);
}