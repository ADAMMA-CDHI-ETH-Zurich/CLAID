
import 'dart:async';
import 'dart:io';


import 'package:claid/CLAID.dart';
import 'package:claid/module/module_factory.dart';
import 'package:flutter/material.dart';
import 'package:path_provider/path_provider.dart';

import 'CLAIDModuleListView.dart';
import 'CLAIDModuleView.dart';
import 'CLAIDModuleViewToClassMap.dart';
import 'package:claid/package/CLAIDPackage.dart';
import 'package:claid/package/CLAIDPackageLoader.dart';



class CLAIDView extends StatefulWidget 
{
  const CLAIDView({super.key, required this.title});

  final String title;

  @override
  State<CLAIDView> createState() => _CLAIDViewState();
}

class _CLAIDViewState extends State<CLAIDView>
{
  bool _claidStarted = false;

  CLAIDModuleListView? moduleListView = null;

  @override
  void initState() 
  {
    super.initState();

    CLAID.getRunningModules().then((modules) => _createDeviceView(modules!));
  }

  void _createDeviceView(Map<String, String> modules)
  {
    setState(() {
      moduleListView = CLAIDModuleListView(title: this.widget.title,
          moduleViewClasses: CLAIDModuleViewToClassMap.getMap(),
          runningModules: modules,
          moduleManager: CLAID.getModuleManager()!,);
    });
  }

  @override
  Widget build(BuildContext context)
  {
    return moduleListView != null ? moduleListView! : CircularProgressIndicator();
  }
}
