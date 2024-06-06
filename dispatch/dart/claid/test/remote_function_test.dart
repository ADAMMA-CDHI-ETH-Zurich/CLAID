import 'package:flutter_test/flutter_test.dart';

import 'dart:async';
import 'dart:io';

import 'package:claid/RemoteFunction/RPCCompleter.dart';

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

}

void main() {
  test('test remote functions', () async {
    runTest();
  });

}