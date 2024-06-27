import 'package:claid/module/module_factory.dart';
import 'package:claid/ui/CLAIDView.dart';
import 'package:claid/CLAID.dart';

import 'package:flutter/material.dart';

import 'TestStreamModule.dart';
import 'TestStreamView.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget
{
  const MyApp({super.key});

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'CLAID App',
      theme: ThemeData(
        useMaterial3: true,
      ),
      home: const MyHomePage(),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key});

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage>
{
  bool claidStarted = false;

  @override
  void initState() {
    super.initState();

    CLAID.startInBackground(
        configFilePath: "assets://flutter_assets/assets/claid_test.json",
        hostId: "Smartphone",
        userId: "test_user",
        deviceId: "test_id",
        specialPermissionsConfig: CLAIDSpecialPermissionsConfig.regularConfig(),
        persistanceConfig: CLAIDPersistanceConfig.maximumPersistance(),
        claidPackages: []
    ).then((value) => setState(() {
      claidStarted = true;
    }));
  }

  @override
  Widget build(BuildContext context)
  {
    return  claidStarted ?
      const CLAIDView(title: 'My CLAID App') :
      const CircularProgressIndicator();
  }
}
