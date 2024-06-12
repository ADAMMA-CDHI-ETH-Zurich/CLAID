
import 'dart:async';
import 'dart:io';


import 'package:claid/RemoteFunction/RemoteFunctionHandler.dart';
import 'package:claid/ui/CLAIDModuleViewToClassMap.dart';
import 'package:claid/ui/EmptyDefaultDeviceView.dart';
import 'package:flutter/material.dart';

import 'CLAIDModuleList.dart';
import 'CLAIDModuleView.dart';

class CLAIDModuleListView extends StatefulWidget {


  const CLAIDModuleListView({super.key,
      required this.title, required this.moduleViewClasses, required this.runningModules, required this.remoteFunctionHandler});

  final Map<String, ViewFactoryFunc> moduleViewClasses;
  final Map<String, String> runningModules;
  final RemoteFunctionHandler remoteFunctionHandler;

  final String title;

  @override
  State<CLAIDModuleListView> createState() => _CLAIDModuleListViewState();
}

class _CLAIDModuleListViewState extends State<CLAIDModuleListView>
{

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
      return CLAIDModuleViewToClassMap().getView(entry.key, entry.value, this.widget.remoteFunctionHandler);
    }));

    claidModuleList = CLAIDModuleList(modules: _deviceViews, onPressed: (val){
      print("OnPressed ${val}");
      pageController.jumpToPage(val + 1);

    });


    _pages = [
      claidModuleList!];//_galaxyWatchView, _smartInhalerDeviceView, _smartInhalerDeviceView];

    _pages.addAll(_deviceViews);

    uiUpdateTimer = Timer.periodic(const Duration(seconds: 5), (Timer t) {
      claidModuleList!.update();
    });


  }

  int _currentIndex = 0;
  final pageController = PageController();
  void onPageChanged(int index) {
    setState(() {
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
          title: Text(widget.title),
          leading: Visibility(child:BackButton(onPressed: (){
            pageController.jumpToPage(0);

          } ), visible: _currentIndex > 0, )
      ),
      body:
      PageView(
        children: _pages,
        controller: pageController,
        onPageChanged: onPageChanged,
      ),




    );

    /*return Scaffold(
      appBar: AppBar(
        // TRY THIS: Try changing the color here to a specific color (to
        // Colors.amber, perhaps?) and trigger a hot reload to see the AppBar
        // change color while the other colors stay the same.
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        // Here we take the value from the MyHomePage object that was created by
        // the App.build method, and use it to set our appbar title.
        title: Text(widget.title),
      ),
      body: Center(
        // Center is a layout widget. It takes a single child and positions it
        // in the middle of the parent.
        child: ListView(
          children: widgets, // Display the list of widgets here
        ),
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: _incrementCounter,
        tooltip: 'Increment',
        child: const Icon(Icons.add),
      ), // This trailing comma makes auto-formatting nicer for build methods.
    );*/
  }


}
