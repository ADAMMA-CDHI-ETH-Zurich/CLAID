import 'package:flutter_test/flutter_test.dart';

import 'dart:async';
import 'dart:io';

import 'package:claid/RemoteFunction/RPCCompleter.dart';
import 'package:claid/CLAID.dart';
import 'package:claid/module/channel.dart';
import 'package:claid/module/module.dart';
import 'package:claid/module/properties.dart';
import 'package:claid/module/module_factory.dart';
import 'package:claid/generated/claidservice.pb.dart';

import 'package:claid/RemoteFunction/RemoteFunction.dart';
import 'package:claid/RemoteFunction/RemoteFunctionHandler.dart';
import 'package:claid/module/module_manager.dart';

import 'package:claid/ui/MappedModule.dart';


class TestStreamer extends Module
{

    late Channel<String> testStream;
    int ctr = 0;
    void initialize(Properties properties)
    {
        moduleInfo("Initialize");
        testStream = publish<String>("TestStream", "");

        final targetTime = DateTime.now().add(Duration(milliseconds: 1000));

        registerPeriodicFunction("DataStream", Duration(milliseconds: 250), _periodicFunction);
    }

    void _periodicFunction()
    {
      print("Function");
      testStream.post("Test " + ctr.toString());
      ctr++;
    }
}

class TestReceiver extends MappedModule
{
    TestReceiver(String entityName, String mappedModuleId,
    String moduleClass, RemoteFunctionHandler remoteFunctionHandler, 
    ModuleManager moduleManager) : super(entityName, mappedModuleId, moduleClass, remoteFunctionHandler, moduleManager)
    {
      subscribeModuleChannel("TestStream", "", onStreamData).then((data) => onSubscriptionResult);
    }

    void onSubscriptionResult(bool result)
    {

    }

    void onStreamData(String data)
    {
      print("on data " + data);
    }
}

void runTest() async
{
  

  await startCLAID();

  ModuleManager manager = CLAID.getModuleManager()!;
  RemoteFunctionHandler handler = manager.getRemoteFunctionHandler();
  TestReceiver receiver = TestReceiver("TestStreamReceiver", "TestStreamer", "TestStreamer", handler, manager);
}

Future<void> startCLAID() async
{
    ModuleFactory moduleFactory = ModuleFactory();
    moduleFactory.registerClass("TestStreamer", () => TestStreamer());

    await CLAID.start("/tmp/loose_direct_subscription_test_grpc_server.sock", 
      "test/loose_direct_subscription_test.json", "test_client",
      "some_user", "some_device", moduleFactory);

   
}

void main() {
  test('test remote functions', () async {
    runTest();
    await Future.delayed(Duration(seconds: 4));
   

  });

}