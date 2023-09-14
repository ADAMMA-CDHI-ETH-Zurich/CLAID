import 'package:flutter_test/flutter_test.dart';

import 'package:claid_core/module.dart';

// TODO: implement module
class MyTestModule extends Module {
  @override
  void initialize(Map<String, String> properties) {
    // TODO: implement initialize
  }
}

void main() {
  test('test basic module', () async {
    Module.registerModule("mytestmodule.dart", () => MyTestModule());

    // TODO: Write test harness for modules !
  });
}
