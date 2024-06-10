import 'package:flutter_test/flutter_test.dart';

import 'dart:async';
import 'dart:io';

import 'package:claid/RemoteFunction/RPCCompleter.dart';
import 'package:claid/CLAID.dart';
import 'package:claid/module/module.dart';
import 'package:claid/module/properties.dart';
import 'package:claid/module/module_factory.dart';

import 'package:claid/RemoteFunction/RemoteFunction.dart';

bool rpcCorrect = false;

class TestClass
{
  String prefix;

  TestClass(this.prefix)
  {

  }

  String testFunction(int a, String b)
  {
    return prefix + " " + b + " " + a.toString();
  }
}

class RPCCaller extends Module
{

    RemoteFunction<int>? mappedFunction = null;

    void initialize(Properties properties)
    {
        moduleInfo("Intialize");
        mappedFunction = mapRemoteFunctionOfModule<int>("RPCCallee", "calculate_sum", 42, ["", ""]);

        final targetTime = DateTime.now().add(Duration(milliseconds: 1000));
        registerScheduledFunction("TestFunction", targetTime, _scheduledFunction);

    }

    void _scheduledFunction() async
    {
      print("Scheduled function called");
      int result = (await mappedFunction!.execute(["5", "42"]))!;
      print("Got result " + result.toString());

      if(result == 5 + 42)
      {
          rpcCorrect = true;
      }
      else
      {
          var errorMessage = "Invalid result! Expected 47 but got " + result.toString();
          print(errorMessage);
      }
    }

   

    
}

class RPCCallee extends Module
{
    void initialize(Properties properties)
    {
        moduleInfo("RPCCalle init");
        registerRemoteFunction<int>("calculate_sum", calculateSum, 42, ["", ""]);
    }

    int calculateSum(String a, String b)
    {
        print("Received data " + a + " " + b);
        int result = int.parse(a) + int.parse(b);
        print("Result " + result.toString());
        return result;
    }
}


void runTest() async
{
  TestClass a = new TestClass("Tst 1");
  TestClass b = new TestClass("TestClass");

  Function func = a.testFunction;
  Function funcb = b.testFunction;

  String result = func(2, "tst");
  print("Result is ${result}");

  result = funcb(40, "test");
  print("Result is ${result}");

  List<dynamic> parameters = [42, "listtest"];

  print("post");
  result = Function.apply(funcb, parameters);
  print("Result is ${result}");
  print("pre");

  dynamic x = "42";
  dynamic y = "Hello";

  var z = x.runtimeType == y.runtimeType;
  print("Result ${z}");

  await startCLAID();
}

Future<void> startCLAID() async
{
    ModuleFactory moduleFactory = ModuleFactory();
    moduleFactory.registerClass("RPCCaller", () => RPCCaller());
    moduleFactory.registerClass("RPCCallee", () => RPCCallee());

    CLAID.start("/tmp/test_grpc_server.sock", 
      "test/remote_function_test.json", "test_client",
      "some_user", "some_device", moduleFactory);

   
}

void main() {
  test('test remote functions', () async {
    runTest();
    await Future.delayed(Duration(seconds: 4));
    expect(rpcCorrect, true);
  });

}