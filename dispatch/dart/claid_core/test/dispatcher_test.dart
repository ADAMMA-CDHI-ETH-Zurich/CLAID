import 'package:claid_core/mocks.dart';
import 'package:flutter_test/flutter_test.dart';

import 'package:claid_core/module.dart';

// TODO: implement module
class MyTestModule extends Module {
  @override
  void initialize(Map<String, String> properties) {
    // TODO: implement initialize
  }

  @override
  String get className => 'MyTestModuleClass';
}

void main() {
  test('test basic module', () async {
    final dispatcher = MockDispatcher();
    final moduleFactories = <String, FactoryFunc>{
      'MyTestModule': () => MyTestModule(),
    };

    initModules(dispatcher: dispatcher, moduleFactories: moduleFactories);

    // TODO: Write test harness for modules !
  });
}
