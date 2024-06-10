
import 'dart:async';
import 'dart:io';


import 'package:claid_flutter_demo/EmptyDefaultDeviceView.dart';
import 'package:flutter/material.dart';

import 'CLAIDModuleList.dart';
import 'CLAIDModuleView.dart';

class CLAIDDeviceView extends StatefulWidget {
  const CLAIDDeviceView({super.key, required this.title});

  // This widget is the home page of your application. It is stateful, meaning
  // that it has a State object (defined below) that contains fields that affect
  // how it looks.

  // This class is the configuration for the state. It holds the values (in this
  // case the title) provided by the parent (in this case the App widget) and
  // used by the build method of the State. Fields in a Widget subclass are
  // always marked "final".

  final String title;

  @override
  State<CLAIDDeviceView> createState() => _CLAIDDeviceViewState();
}

class _CLAIDDeviceViewState extends State<CLAIDDeviceView>
{

  CLAIDModuleList? claidModuleList;

  CLAIDModuleView view =
    EmptyDefaultDeviceView(imagePath: "assets/images/img.png", mappedModuleId: '', moduleClass: '', remoteFunctionHandler: null,);
  List<Widget> _pages = [];
  List<CLAIDModuleView> _deviceViews = [];

  Timer? uiUpdateTimer;

  @override
  void initState()
  {
    super.initState();
    _deviceViews = [view];

    print("CLAIDDeviceView InitState");
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
