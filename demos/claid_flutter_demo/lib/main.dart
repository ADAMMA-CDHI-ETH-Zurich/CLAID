import 'package:claid/module/module_factory.dart';
import 'package:claid/ui/CLAIDView.dart';
import 'package:claid/CLAID.dart';

import 'package:flutter/material.dart';

import 'TestStreamModule.dart';
import 'TestStreamView.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
        useMaterial3: true,
      ),
      home: const MyHomePage(title: 'Flutter Demo Home Page'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});


  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  int _counter = 0;
  ModuleFactory moduleFactory = ModuleFactory();

  @override
  void initState() {
    super.initState();

    moduleFactory.registerClass("TestStreamModule", () => TestStreamModule());
    CLAID.registerViewClassForModule("TestStreamModule", TestStreamView.new);
  }


  @override
  Widget build(BuildContext context) {

    return MaterialApp(
      home: CLAIDView(title: 'My CLAID App',

          configPath: "assets://flutter_assets/assets/claid_test.json",
          claidPackages: [
          ],
        ),
    );
  }
}
