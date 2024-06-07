import 'dart:io';

import 'package:flutter/material.dart';
import 'package:claid/module/channel_data.dart';
import 'package:claid/generated/claidservice.pb.dart';

class MappedModule
{
    final String _mappedModuleId;
    final String _moduleClass;
    final RemoteFunctionHandler _remoteFunctionHandler;

    RemoteFunction<T> mapFunction(String functionName, T returnType, List<dynamic> parameters)
    {
        return _remoteFunctionHandler.mapModuleFunction(this._mappedModuleId, returnType, parameters);
    }


    RemoteFunction<T> mapMiddlewareFunction(String functionName, T returnType, List<dynamic> parameters)
    {
        return _remoteFunctionHandler.mapRuntimeFunction(Runtime.RUNTIME_CPP, functionName, returnType, parameters);
    }

    Future<bool> subscribeChannel<T>(String channelName, T dataType, ChannelCallback<T> callback) async
    {
        RemoteFunction<bool> function = mapFunction<bool>("soft_subscribe_data", bool(), [String()]);

        return function.execute(channelName);
    }

}