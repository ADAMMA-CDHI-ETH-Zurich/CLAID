import 'dart:io';

import 'package:claid/module/channel.dart';
import 'package:flutter/material.dart';
import 'package:claid/module/channel_data.dart';
import 'package:claid/generated/claidservice.pb.dart';
import 'package:claid/RemoteFunction/RemoteFunction.dart';
import 'package:claid/RemoteFunction/RemoteFunctionHandler.dart';

class MappedModule
{
  final String _mappedModuleId;
  final String _moduleClass;
  final RemoteFunctionHandler _remoteFunctionHandler;

  MappedModule(this._mappedModuleId, this._moduleClass, this._remoteFunctionHandler)
  {

  }

  RemoteFunction<T> mapFunction<T>(String functionName, T returnType, List<dynamic> parameterDataTypes)
  {
    return _remoteFunctionHandler.mapModuleFunction(this._mappedModuleId, functionName, returnType, parameterDataTypes);
  }

  RemoteFunction<T> mapMiddlewareFunction<T>(String functionName, T returnType, List<dynamic> parameters)
  {
    return _remoteFunctionHandler.mapRuntimeFunction(Runtime.RUNTIME_CPP, functionName, returnType, parameters);
  }

  /*
  Future<bool> subscribeChannel<T>(String channelName, T dataType, ChannelCallback<T> callback) async
  {
   // RemoteFunction<bool> function = mapFunction<bool>("soft_subscribe_data", bool(), [String()]);

    return function.execute(channelName);
  }*/

}