
import 'dart:async';
import 'dart:io';


import 'package:claid/CLAID.dart';
import 'package:claid/module/module_factory.dart';
import 'package:flutter/material.dart';
import 'package:path_provider/path_provider.dart';

import 'CLAIDDeviceView.dart';
import 'TestStreamModule.dart';




class CLAIDAppPage extends StatefulWidget {
  const CLAIDAppPage({super.key, required this.title});

  // This widget is the home page of your application. It is stateful, meaning
  // that it has a State object (defined below) that contains fields that affect
  // how it looks.

  // This class is the configuration for the state. It holds the values (in this
  // case the title) provided by the parent (in this case the App widget) and
  // used by the build method of the State. Fields in a Widget subclass are
  // always marked "final".

  final String title;

  @override
  State<CLAIDAppPage> createState() => _CLAIDAppPageState();
}

class _CLAIDAppPageState extends State<CLAIDAppPage>
{

  bool _claidStarted = false;

  late CLAIDDeviceView deviceView;

  @override
  void initState() {
    super.initState();
    print("CLAIDAppPage InitState");
  }

  @override
  Widget build(BuildContext context)
  {
    deviceView = CLAIDDeviceView(title: this.widget.title);
    print("Dart ALEX main 4");

    if(!_claidStarted)
    {
      print("Dart ALEX main 5");

      startCLAID();
      _claidStarted = true;
    }
    // This method is rerun every time setState is called, for instance as done
    // by the _incrementCounter method above.
    //
    // The Flutter framework has been optimized to make rerunning build methods
    // fast, so that you can just rebuild anything that needs updating rather
    // than having to individually change instances of widgets.



    return deviceView;
  }

  void startCLAID() async
  {
    Directory? appDocDir = await getApplicationDocumentsDirectory();
    print("Dart ALEX main 6");

    // Construct the path to the Android/media directory
    String mediaDirectoryPath = '${appDocDir!.path}';

    mediaDirectoryPath = mediaDirectoryPath.replaceAll("app_flutter", "files");
    String socketPath = mediaDirectoryPath + "/" + "claid_local.grpc";

    ModuleFactory moduleFactory = ModuleFactory();
    moduleFactory.registerClass("TestStreamModule", () => TestStreamModule());
    print("ATTACHING DART RUNTIME " + socketPath + "\n");
    //moduleFactory.registerClass("MyTestModuleOne", () => MyTestModuleOne());
    CLAID.start(socketPath,
    "assets/claid_test.json", "test_host", "test_user", "test_id", moduleFactory);
    //CLAID.attachDartRuntime("unix://" + socketPath, moduleFactory);
    print("Dart ALEX main 7");
  }
}
