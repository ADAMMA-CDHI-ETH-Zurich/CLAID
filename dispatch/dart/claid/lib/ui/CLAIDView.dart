
import 'dart:async';
import 'dart:io';


import 'package:claid/CLAID.dart';
import 'package:claid/module/module_factory.dart';
import 'package:flutter/material.dart';
import 'package:path_provider/path_provider.dart';

import 'CLAIDModuleListView.dart';
import 'CLAIDModuleView.dart';
import 'CLAIDModuleViewToClassMap.dart';




class CLAIDView extends StatefulWidget {
  const CLAIDView({super.key,
    required this.title, required this.moduleFactory});

  // This widget is the home page of your application. It is stateful, meaning
  // that it has a State object (defined below) that contains fields that affect
  // how it looks.

  // This class is the configuration for the state. It holds the values (in this
  // case the title) provided by the parent (in this case the App widget) and
  // used by the build method of the State. Fields in a Widget subclass are
  // always marked "final".

  final String title;

  final ModuleFactory moduleFactory;

  @override
  State<CLAIDView> createState() => _CLAIDViewState();

  static void registerViewClassForModule(String moduleClass, ViewFactoryFunc factoryFunc)
  {
    CLAIDModuleViewToClassMap.registerModuleClass(moduleClass, factoryFunc);
  }
}

class _CLAIDViewState extends State<CLAIDView>
{
  bool _claidStarted = false;

  CLAIDModuleListView? moduleListView = null;

  @override
  void initState() {
    super.initState();
    print("CLAIDAppPage InitState");
  }

  @override
  Widget build(BuildContext context)
  {


    if(!_claidStarted)
    {
      startCLAID();
      _claidStarted = true;
      return CircularProgressIndicator();
    }
    // This method is rerun every time setState is called, for instance as done
    // by the _incrementCounter method above.
    //
    // The Flutter framework has been optimized to make rerunning build methods
    // fast, so that you can just rebuild anything that needs updating rather
    // than having to individually change instances of widgets.



    return moduleListView != null ? moduleListView! : CircularProgressIndicator();
  }

  void _createDeviceView(Map<String, String> modules)
  {
    setState(() {
      moduleListView = CLAIDModuleListView(title: this.widget.title,
          moduleViewClasses: CLAIDModuleViewToClassMap.getMap(),
          runningModules: modules,
          remoteFunctionHandler: CLAID.getRemoteFunctionHandler());
    });



  }

  void startCLAID() async
  {
    Directory? appDocDir = await getApplicationDocumentsDirectory();

    // Construct the path to the Android/media directory
    String mediaDirectoryPath = '${appDocDir!.path}';

    mediaDirectoryPath = mediaDirectoryPath.replaceAll("app_flutter", "files");
    String socketPath = mediaDirectoryPath + "/" + "claid_local.grpc";


    print("ATTACHING DART RUNTIME " + socketPath + "\n");
    //moduleFactory.registerClass("MyTestModuleOne", () => MyTestModuleOne());

    print("Current ${Directory.current.path}");
    await CLAID.start(socketPath,
    "assets/claid_test.json", "test_host", "test_user", "test_id",
        this.widget.moduleFactory, libraryPath: "/Users/planger/Development/CLAID/dispatch/dart/claid/blobs/libclaid_capi.dylib");
    //CLAID.attachDartRuntime("unix://" + socketPath, moduleFactory);
    print("Dart ALEX main 7");

    CLAID.getRunningModules().then((modules) => _createDeviceView(modules!));
  }
}