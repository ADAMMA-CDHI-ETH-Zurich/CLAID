
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
    List sortedEntries = modules.entries.toList()
      ..sort((a, b) => a.key.compareTo(b.key));

    // Create a new map from the sorted entries
    Map<String, String> sortedModules = {
      for (var entry in sortedEntries) entry.key: entry.value
    };

    setState(() {
      moduleListView = CLAIDModuleListView(
          title: this.widget.title,
          moduleViewClasses: CLAIDModuleViewToClassMap.getMap(),
          runningModules: sortedModules,
          moduleManager: CLAID.getModuleManager()!
        );
    });
  }

  @override
  Widget build(BuildContext context)
  {
    return moduleListView != null ? moduleListView! : CircularProgressIndicator();
  }
}
