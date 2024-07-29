
import 'dart:async';
import 'dart:io';


import 'package:claid/RemoteFunction/RemoteFunctionHandler.dart';
import 'package:claid/module/module_manager.dart';

import 'package:claid/ui/CLAIDModuleViewToClassMap.dart';
import 'package:claid/ui/EmptyDefaultDeviceView.dart';
import 'package:flutter/material.dart';

import 'CLAIDModuleList.dart';
import 'CLAIDModuleView.dart';

class CLAIDModuleListView extends StatefulWidget {


  const CLAIDModuleListView({super.key,
      required this.title, required this.moduleViewClasses,
      required this.runningModules, required this.moduleManager});

  final Map<String, ViewFactoryFunc> moduleViewClasses;
  final Map<String, String> runningModules;
  final ModuleManager moduleManager;

  final String title;

  @override
  State<CLAIDModuleListView> createState() => _CLAIDModuleListViewState();
}

class _CLAIDModuleListViewState extends State<CLAIDModuleListView>
{

  int currentPage = -1;

  CLAIDModuleList? claidModuleList;

  List<Widget> _pages = [];
  List<CLAIDModuleView> _deviceViews = [];

  Timer? uiUpdateTimer;

  @override
  void initState()
  {
    super.initState();


    print("CLAIDModuleListView InitState");

    _deviceViews.addAll(this.widget.runningModules.entries.map((entry) {
      return CLAIDModuleViewToClassMap().getView(entry.key, entry.value, this.widget.moduleManager);
    }));

    claidModuleList = CLAIDModuleList(modules: _deviceViews, onPressed: (val){
      print("OnPressed ${val}");
      
      
      
      pageController.jumpToPage(val + 1);

    });


    _pages = [claidModuleList!];
    _pages.addAll(_deviceViews);

    uiUpdateTimer = Timer.periodic(const Duration(seconds: 5), (Timer t) {
      claidModuleList!.update();
    });


  }

  int _currentIndex = 0;
  final pageController = PageController();
  void onPageChanged(int index) {
    setState(() {
      if(_currentIndex != 0)
      {
        _deviceViews[_currentIndex - 1].onHidden();   
      }
      if(index != 0)
      {
        _deviceViews[index - 1].onShown();
      }
      _currentIndex = index;
    });

    claidModuleList!.update();
  }

  @override
  Widget build(BuildContext context)
  {
    return Scaffold(
      appBar: AppBar(
        // Here we take the value from the MyHomePage object that was created by
        // the App.build method, and use it to set our appbar title.
          title: Text(widget.title,
            textAlign: TextAlign.center,
            style: const TextStyle(color:Colors.white),),
            centerTitle: true,
            backgroundColor: Color.fromARGB(255, 107, 180, 236),
            leading: Visibility(child:BackButton(onPressed: (){
              pageController.jumpToPage(0);

            } ),
            visible: _currentIndex > 0, )
      ),
      body: Padding(padding: const EdgeInsets.only(top: 10),
          child: PageView(
            children: _pages,
            controller: pageController,
            onPageChanged: onPageChanged,
          ),)
    );
  }
}
