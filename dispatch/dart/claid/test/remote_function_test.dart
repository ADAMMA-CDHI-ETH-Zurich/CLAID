import 'package:flutter_test/flutter_test.dart';

import 'dart:async';
import 'dart:io';

import 'package:claid/RemoteFunction/RPCCompleter.dart';
import 'package:claid/CLAID.dart';
import 'package:claid/module/module.dart';
import 'package:claid/module/properties.dart';
import 'package:claid/module/module_factory.dart';
import 'package:claid/generated/claidservice.pb.dart';

import 'package:claid/RemoteFunction/RemoteFunction.dart';

bool rpcCorrect = false;
bool voidRPCParameterCorrect = false;
bool voidRPCReturnCorrect = false;
bool voidRPCVoidCorrect = false;

Map<String, String> runningModulesRPCResult = {};

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


    void initialize(Properties properties)
    {
        moduleInfo("Intialize");

        final targetTime = DateTime.now().add(Duration(milliseconds: 1000));
        registerScheduledFunction("ModuleRPCTest", targetTime, _moduleRPCTest);
        registerScheduledFunction("RuntimeRPCTest", targetTime, _runtimeRPCTest);
        registerScheduledFunction("VoidRPCTest", targetTime, _voidRPCTest);

    }

    void _moduleRPCTest() async
    {
      RemoteFunction<int>? mappedFunction = mapRemoteFunctionOfModule<int>("RPCCallee", "calculate_sum", 42, ["", ""]);
      print("Scheduled function called");
      int result = (await mappedFunction!.executeWithParameters(["5", "42"]))!;
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

    void _runtimeRPCTest() async
    {
        print("Executing RuntimeRPCTest");
        RemoteFunction<Map<String, String>>? mappedFunction = 
          mapRemoteFunctionOfRuntime<Map<String, String>>(Runtime.MIDDLEWARE_CORE, "get_all_running_modules_of_all_runtimes", Map<String, String>(), []);

        runningModulesRPCResult = (await mappedFunction!.executeWithParameters([]))!;
    }

    void _voidRPCTest() async
    {
      RemoteFunction<void> voidReturnFunction =
        mapRemoteFunctionOfModule("RPCCallee", "void_return_function", null, [0]);

      RemoteFunction<int> voidParameterFunction =
        mapRemoteFunctionOfModule("RPCCallee", "void_parameter_function", 0, []);

      RemoteFunction<void> voidVoidFunction =
        mapRemoteFunctionOfModule("RPCCallee", "void_void_function", null, []);

      (voidReturnFunction.executeWithParameters([1337]))!;

      int result = (await voidParameterFunction.executeWithParameters([]))!;
      if(result == 1338)
      {
        voidRPCReturnCorrect = true;
      }

      await voidVoidFunction.executeWithParameters([]);
    }

   

    
}

class RPCCallee extends Module
{
    void initialize(Properties properties)
    {
        moduleInfo("RPCCalle init");
        registerRemoteFunction<int>("calculate_sum", calculateSum, 42, ["", ""]);

        registerRemoteFunction<void>("void_return_function", voidReturnFunction, null, [0]);
        registerRemoteFunction<int>("void_parameter_function", voidParameterFunction, 0, []);
        registerRemoteFunction<void>("void_void_function", voidVoidFunction, null, []);
    }

    int calculateSum(String a, String b)
    {
        print("Received data " + a + " " + b);
        int result = int.parse(a) + int.parse(b);
        print("Result " + result.toString());
        return result;
    }

    void voidReturnFunction(int magic)
    {
      if(magic == 1337)
      {
        voidRPCParameterCorrect = true;
      }
    }

    int voidParameterFunction()
    {
       return 1338;
    }

    void voidVoidFunction()
    {
      voidRPCVoidCorrect = true;
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
    CLAID.registerModule("RPCCaller", () => RPCCaller());
    CLAID.registerModule("RPCCallee", () => RPCCallee());

    CLAID.startMiddleware("/tmp/remote_function_test_grpc_server.sock", 
      "test/remote_function_test.json", "test_client",
      "some_user", "some_device", CLAIDSpecialPermissionsConfig.regularConfig());

   
}

void main() {
  test('test remote functions', () async {
    runTest();
    await Future.delayed(Duration(seconds: 4));
    expect(rpcCorrect, true);
    expect(runningModulesRPCResult.length, 2);
    expect(runningModulesRPCResult["RPCCaller"], "RPCCaller");
    expect(runningModulesRPCResult["RPCCallee"], "RPCCallee");

    expect(voidRPCParameterCorrect, true);
    expect(voidRPCReturnCorrect, true);
    expect(voidRPCVoidCorrect, true);

  });

}